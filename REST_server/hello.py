'''
This Flask application exposes a RESTful API for interfacing with a BMP280 sensor connected to an STM32 microcontroller. It manages:
    - Temperature: Reading and storing temperature values.
    - Pressure: Reading and storing pressure values.
'''

import json
from flask import Flask
import serial

from flask import jsonify
from flask import render_template
from flask import abort
from flask import request
import serial

app = Flask(__name__)


tab_T = []  # Array for temperatures
tab_P = []  # Array for pressures


##########################
# Core application pages #
##########################
ser = serial.serial_for_url(f'socket://192.168.88.235:5000', timeout=1)


@app.route('/')
def hello_world():
    return jsonify({"message": 'Hello, World!'})

welcome = "Welcome to 3ESE API!"

@app.route('/api/welcome/')
def api_welcome():
    if request.method == 'GET' :
        return jsonify({"phrase" : welcome})

@app.route('/api/welcome/<int:index>', methods=['GET','POST','DELETE'])
def api_welcome_index(index):
    if index < 0 or index >= len(welcome):
        abort(404)

    if request.method == 'GET' :
        return jsonify({"index" : index, "value" : welcome[index]})

    #elif request.method == 'POST' :

    #return jsonify({"index": index, "val": welcome[index]})

def get_temp() :
    try:
        ser.write(b'GET_T\n')  # Envoie la commande série "GET_T" pour obtenir la température
        temp = ser.readline().decode().strip()  # Lit et décode la réponse
        return temp
    except serial.SerialException as e:
        print(f"Erreur de connexion série pour la température : {e}")
        return None


def get_pres() :
    try:
        ser.write(b'GET_P\n')  # Envoie la commande série "GET_P" pour obtenir la pression
        pres = ser.readline().decode().strip()  # Lit et décode la réponse
        return pres
    except serial.SerialException as e:
        print(f"Erreur de connexion série pour la pression : {e}")
        return None


@app.route('/api/<path>', methods=['GET','POST','DELETE'])
def api_data(path) :
    if path == 'temp' and request.method == 'GET' :
        temp = get_temp()
        if temp is not None :
            return jsonify({"temperature" : temp})
        else :
            return jsonify({"error" : "get_temp() ne fonctionne pas."})

    elif path == 'pres' and request.method == 'GET' :
        pres = get_pres()
        if pres is not None :
            return jsonify({"pressure" : pres})
        else :
            return jsonify({"error" : "get_pres() ne fonctionne pas."})

    else :
        return jsonify({"error" : "path inconnu."})









@app.errorhandler(404)
def page_not_found(error):
    return render_template('page_not_found.html'), 404

@app.route('/api/request/', methods=['GET', 'POST'])
@app.route('/api/request/<path>', methods=['GET','POST'])
def api_request(path=None):
    resp = {
            "method":   request.method,
            "url" :  request.url,
            "path" : path,
            "args": request.args,
            "headers": dict(request.headers),
    }
    if request.method == 'POST':
        resp["POST"] = {
                "data" : request.get_json(),
                }
    return jsonify(resp)


################################
# Communication with the STM32 #
################################
'''
ser = serial.Serial("/dev/ttyAMA0",115200,timeout=1)
ser.reset_output_buffer()
ser.reset_input_buffer()

# Temperature endpoint
@app.route('/api/temp/', methods=['GET', 'POST'])
def api_temp():
    ser.reset_output_buffer()
    ser.reset_input_buffer()
    resp = {
        "method": request.method,
        "url": request.url,
        "args": request.args,
        "headers": dict(request.headers),
    }
    if request.method == 'POST':
        ser.write(b'GET_T')  # Sends to the STM32 that we want to perform a GET_T
        tempo = ser.readline().decode()  # Retrieve the value sent by the STM32
        tab_T.append(tempo[:9])  # Remove '\r\n' and add it to the array
        return jsonify(tab_T[-1])  # Return the last value
    if request.method == 'GET':
        return jsonify(tab_T)  # Return the entire temperature array

@app.route('/api/temp/<int:index>', methods=['GET', 'DELETE'])
def api_temp_index(index=None):
    resp = {
        "method": request.method,
        "url": request.url,
        "index": index,
        "args": request.args,
        "headers": dict(request.headers),
    }
    if request.method == 'GET':
        if index < len(tab_T):
            return jsonify(tab_T[index])  # Retrieve the value from the array at the index
        else:
            return jsonify("error: index out of range")
    if request.method == 'DELETE':
        if index < len(tab_T):
            return jsonify(f"The value {tab_T.pop(index)} has been removed")  # Remove value from the array
        else:
            return jsonify("error: index out of range")


# Pressure endpoint
@app.route('/api/pres/', methods=['GET', 'POST'])
def api_pres():
    ser.reset_output_buffer()
    ser.reset_input_buffer()
    resp = {
        "method": request.method,
        "url": request.url,
        "args": request.args,
        "headers": dict(request.headers),
    }
    if request.method == 'POST':
        ser.write(b'GET_P')  # Sends to the STM32 that we want to perform a GET_P
        tempo = ser.readline().decode()  # Retrieve the value sent by the STM32
        tab_P.append(tempo[:20])  # Remove '\r\n' and add it to the array
        return jsonify(tab_P[-1])  # Return the last value
    if request.method == 'GET':
        return jsonify(tab_P)  # Return the entire pressure array

@app.route('/api/pres/<int:index>', methods=['GET', 'DELETE'])
def api_pres_index(index=None):
    resp = {
        "method": request.method,
        "url": request.url,
        "index": index,
        "args": request.args,
        "headers": dict(request.headers),
    }
    if request.method == 'GET':
        if index < len(tab_P):
            return jsonify(tab_P[index])  # Retrieve the value from the array at the index
        else:
            return jsonify("error: index out of range")
    if request.method == 'DELETE':
        if index < len(tab_P):
            return jsonify(f"The value {tab_P.pop(index)} has been removed")  # Remove value from the array
        else:
            return jsonify("error: index out of range")
            
'''
