import serial.tools.list_ports
import re
import csv
import time
from sklearn.model_selection import train_test_split
import pandas as pd
from keras.models import Sequential
from keras.layers import Dense
from keras.utils import to_categorical
from sklearn.preprocessing import LabelEncoder


ports = serial.tools.list_ports.comports()
serialInst = serial.Serial()
portsList = []

for onePort in ports:
    portsList.append(str(onePort))
    print(str(onePort))

val = input("Select Port: COM")

for x in range(0,len(portsList)):
    if portsList[x].startswith("COM" + str(val)):
        portVar = "COM" + str(val)
        print(portVar)

serialInst.baudrate = 9600
serialInst.port = portVar
serialInst.open()

csv_file_path = 'example.csv'
final_data=[]
data_dict={}
final_data_dict={}



df=pd.read_csv(csv_file_path)

X = df[['latitude','longitude']]
y = df['gas']
X_train, X_test, Y_train, Y_test = train_test_split(X, y, test_size=0.2)
model1 = Sequential()
model1.add(Dense(32, input_dim=2, activation='relu'))
model1.add(Dense(16, activation='relu'))
model1.add(Dense(1, activation='linear'))
model1.compile(loss='mean_squared_error', optimizer='adam')
model1.fit(X_train, Y_train, epochs=150, batch_size=10)


Y = to_categorical(y)  
y_category = ['HIGH','MEDIUM','LOW']
model2 = Sequential()
model2.add(Dense(32, input_dim=2, activation='relu'))
model2.add(Dense(16, activation='relu'))
model2.add(Dense(Y.shape[1], activation='softmax')) 

model2.compile(loss='categorical_crossentropy', optimizer='adam')
model2.fit(X, Y, epochs=10, batch_size=10)
le = LabelEncoder()
y_encoded = le.fit_transform(y_category)
print("Encoded labels: ", y_encoded)
print("Categories of the model: ", le.classes_)



while True:
    try:
        if serialInst.in_waiting:
            packet = serialInst.readline()
            data=packet.decode('utf').rstrip('\r\n')
            key,value=data.split(":")
            key=key.strip()
            value=value.strip()
            data_dict[key]=value
            # humidity_level_pattern=r"\d{2}\.\d{2}\D"
            # temperature_pattern=r'\d{2}.\d{2}.C'
            print(data_dict)
            if data_dict["Location"] !=None:
                if int(data_dict['Analog output'])<8000:
                    latitude=str(float(data_dict['Location'].split(',')[0]))
                    longitude=str(float(data_dict['Location'].split(',')[1]))
                    l1=[latitude,longitude,str(float(data_dict['Analog output'])/10000),data_dict['Humidity'],data_dict['Temperature']]
                    final_data.append(l1)
                    with open(csv_file_path, mode='a', newline='') as file:
                        writer = csv.writer(file)
                        writer.writerows(final_data)
                
                elif int(data_dict['Analog output'])==8191:

                    latitude=str(float(data_dict['Location'].split(',')[0]))
                    longitude=str(float(data_dict['Location'].split(',')[1]))
                    final_data_dict['latitude']=float(latitude)
                    final_data_dict['longitude']=float(longitude)
                    df = pd.DataFrame([final_data_dict])
                    Y_pred = model1.predict(df)
                    print(Y_pred)
                    l1=[str(latitude),str(longitude),str(float(Y_pred[0][0])),data_dict['Humidity'],data_dict['Temperature']]
                    final_data.append(l1)
                    with open(csv_file_path, mode='a', newline='') as file:
                        writer = csv.writer(file)
                        writer.writerows(final_data)

                level_category = model2.predict(X_test)
                if int(level_category[0][0])==1:
                    print("gas level: Medium")
                elif int(level_category[0][0])==0:
                    print("gas level: High")
                elif int(level_category[0][0])==2:
                    print("gas level: Low")
                data_dict={}
                final_data=[]
                final_data_dict={}
                
    except:
        pass
   