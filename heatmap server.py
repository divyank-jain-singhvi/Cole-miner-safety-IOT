from flask import Flask, render_template
import folium
from folium.plugins import HeatMap
import pandas as pd

data=[]
train_data=[]
app = Flask(__name__)

@app.route('/')
def map_display():
    
    mapObj = folium.Map([13.90396,75.63052], zoom_start=5)
    data = get_data()
    HeatMap(data).add_to(mapObj)
    mapObj.save("templates/output.html")
    return render_template('output.html')
 
def get_data():
    global data
    data = []  # Clear the data list
    df=pd.read_csv("example.csv") 
    df.columns = range(df.shape[1])
    for index, row in df.iterrows():
        data.append(row.tolist())
    for i in data:
        train_data.append([i[0],i[1],i[2]])
    return train_data

if __name__ == '__main__':
    while True:
        app.run(debug=True)
  