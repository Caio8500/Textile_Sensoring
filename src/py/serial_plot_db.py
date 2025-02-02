import serial
import re
import csv
from datetime import datetime
import matplotlib.pyplot as plt
import dateutil.parser as dparser
import numpy as np
import influxdb_client, os, time
from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS


def logMeasures(numberOfMesures = 10*16, filename = "measures.csv"):
    """
    This function logs a set of 16 measures + temperature in a csv file and in a InfluxDb cloud database
    We receive data from the arduino from the COM serial port, we then apply a regex on the input to filer out any noise that comes 
    from the serial bus. Then, we log the parsed data into a .csv file and the influxDb database
    *Keep in mind we have, in general, one batch of measures per second
    """
    N_SENSORS = 16
    SERIAL_OPTS = {
        #"port": "/dev/ttyACM0",  # typical name on Linux
        "port": "COM9",  # typical name on Windows
        "baudrate": 115200,  # same value as in the Arduino program
        "bytesize": 8,
        "parity": "N",
        "timeout": 1,  # in seconds
    }
    measuresCounter = 0
    
    regexPatten = r'(?<=Sensor\s)(?P<sensor>\d{1,2})(?:\s*\:\s*)(?P<adcReading>\d{1,4})(?:;\s*Temp:\s*(?P<temperature>\d{1,4}\.\d{1,2}))?'
    # (?<=Sensor\s) -> positive lookbehind: the regex will match strings that are preceeded by the pattern "Sensor "
    # (?P<sensor>\d{1,2}) -> named capturing group: the regex will be composed of a named group "Sensor" that indicates the current sensor
    # (?:\s*\:\s*) -> non capturing group: the regex match will ignore this part of the string, which is composed by zero or more occurences of 
    # Unicode whitespace characters (which includes [ \t\n\r\f\v]
    # (?P<adcReading>\d{1,4}) -> named capturing group: the regex will be composed of a named group "adcReading" that indicates the current adc reading
    # (?=;.*) -> positive lookahead: the regex will match strings that are followed by the pattern ";" and any character that comes after it
    # (?:;\s*Temp:\s*(?P<temperature>\d{1,4}\.\d{1,2}))? -> optional non-capturing group with named group temperature 
    # str structure received from the arduino: Sensor 0:29;ÿSensor 1:29;ÿSensor 2:29;ÿSensor 3:29;ÿSensor 4:29;ÿSensor 5:29;ÿSensor 6:29;ÿSensor 7:29;ÿSensor 8:29;ÿSensor 9:29;ÿSensor 10:29;ÿSensor 11:29;ÿSensor 12:29;ÿSensor 13:29;ÿSensor 14:29;ÿSensor 15:29;Temp: 125.03
    ### if you have any doubts about this regex, please use the regex101.com website ###  

    with serial.Serial(**SERIAL_OPTS) as myserial:
        writeHeader = True # should we write the csv header 
        while(measuresCounter < numberOfMesures):
            ## Treating received data
            myline = myserial.readline()
            myline = myline.decode(encoding='latin-1')  # to get a proper string - arduino serial comm tends to send 
                                                        # out garbage in the form of latin encoding (accents and stuff)
                                                        # https://stackoverflow.com/questions/5552555/unicodedecodeerror-invalid-continuation-byte
            myline = myline.rstrip("\r\n")  # remove final characters
            if(myline): # if line is not empty
                matches = re.finditer(regexPatten, myline)
                print(myline)
                newMesuresRow = dict()
                for match in matches:
                    print("Sensor: "+ match.group("sensor"))
                    print("Reading :" + match.group("adcReading")) 
                    print("----") 
                    # this part will be executed for all sensors at every iteration
                    dictKey = match.group("sensor")
                    dictValue = match.group("adcReading")
                    newMesuresRow[dictKey] = dictValue
                    # whereas this part will only be executed at the end of the measurements
                    if(match.group("temperature") != None):
                        dictKey = 'temperature'
                        newMesuresRow[dictKey] = match.group("temperature") 
                        print("Temperature:" + match.group("temperature"))
                    measuresCounter += 1
                ## Saving received data to csv
                with open(filename, mode='a') as measuresFile:
                    fieldnames = ['timestamp','0', '1', '2', '3', 
                                '4', '5', '6', '7', 
                                '8', '9', '10', '11', 
                                '12', '13', '14', '15', 'temperature']
                    csvDictWriter = csv.DictWriter(measuresFile, fieldnames=fieldnames, delimiter=';', lineterminator = '\n')
                    if (writeHeader):
                        measuresFile.truncate(0) # erasing previous contents from file
                        csvDictWriter.writeheader()
                        writeHeader = not(writeHeader) # if header has been written, stop writing it
                    # add +1 to account for the tempeature field
                    # if we have a full row of measures
                    if (len(newMesuresRow) == N_SENSORS + 1):
                        timeStamp = datetime.now()
                        newMesuresRow["timestamp"] = timeStamp.strftime("%d/%m/%Y %H:%M:%S")
                        csvDictWriter.writerow(newMesuresRow)
                        saveMeasuresToDb(newMesuresRow)

def plotHistfromCsv(sensor = 0, filename = "measures.csv"):
    """
    Plots histogram from a csv file content 
    """
    #TODO: use args to pass multiple sensors as input arg 
    # sensor - which sensor should we plot a histogram from (0-16)
    time = []
    measure = []
  
    with open(filename,'r') as csvfile:
        plots = csv.reader(csvfile, delimiter = ';')
        
        for row in plots:

            if (row[0] == 'timestamp'):
                pass
            else:
                timestamp = dparser.parse(row[0])
                timestamp = timestamp.strftime("%H:%M:%S")
                time.append(timestamp)
                measure.append(int(row[sensor + 1]))
    n, bins, patches = plt.hist(x=measure, bins='auto', color='#0504aa',
                            alpha=0.7, rwidth=0.85)
    plt.grid(axis='y', alpha=0.75)
    plt.xlabel('Mesure')
    plt.ylabel('Frequency')
    plt.title('Sensor '  + str(sensor) + ' Histogram')
    #plt.text(23, 45, r'$\mu=15, b=3$')
    maxfreq = n.max()
    # Set a clean upper y-axis limit.
    plt.ylim(ymax=np.ceil(maxfreq / 10) * 10 if maxfreq % 10 else maxfreq + 10)
    plt.show()

def saveMeasuresToDb(measures):
    """
    This function stores data to influx db, it takes a dict of measures as input argument
    """
    INFLUXDB_TOKEN = "*"
    org = "Proj_Them"
    url = "https://us-east-1-1.aws.cloud2.influxdata.com"
    write_client = influxdb_client.InfluxDBClient(url=url, token=INFLUXDB_TOKEN, org=org)
    bucket="SATI"
    # Define the write api
    write_api = write_client.write_api(write_options=SYNCHRONOUS)
    point = (
    Point("Sensors")
    .tag("location", "Enseirb")
    .field("Sensor 00", int(measures["0"]))
    .field("Sensor 01", int(measures["1"]))
    .field("Sensor 02", int(measures["2"]))
    .field("Sensor 03", int(measures["3"]))
    .field("Sensor 04", int(measures["4"]))
    .field("Sensor 05", int(measures["5"]))
    .field("Sensor 06", int(measures["6"]))
    .field("Sensor 07", int(measures["7"]))
    .field("Sensor 08", int(measures["8"]))
    .field("Sensor 09", int(measures["9"]))
    .field("Sensor 10", int(measures["10"]))
    .field("Sensor 11", int(measures["11"]))
    .field("Sensor 12", int(measures["12"]))
    .field("Sensor 13", int(measures["13"]))
    .field("Sensor 14", int(measures["14"]))
    .field("Sensor 15", int(measures["15"]))
    .field("Temperature", int(float(measures["temperature"])))
    )
    write_api.write(bucket=bucket, org=org, record=point)
    

if __name__=="__main__":
    # print("test")
    logMeasures(numberOfMesures=30*16)
    plotHistfromCsv(sensor=7)