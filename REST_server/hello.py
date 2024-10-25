import json
from flask import jsonify
from flask import Flask
from flask import render_template
from flask import abort
from flask import request
app = Flask(__name__)

@app.route('/')
def hello_world():
	return jsonify({"message": 'Hello, World!'})

welcome = "Welcome to 3ESE API!"

@app.route('/api/welcome/')
def api_welcome():
    return jsonify({"message": welcome})
    
@app.route('/api/welcome/<int:index>', methods=['GET','POST'])
def api_welcome_index(index):
    if index < 0 or index >= len(welcome):
        abort(404)
    return jsonify({"index": index, "val": welcome[index]})

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
