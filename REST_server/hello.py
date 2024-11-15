import json
from flask import Flask
from flask import jsonify
from flask import render_template
from flask import abort
from flask import request
import serial
app = Flask(__name__)
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
