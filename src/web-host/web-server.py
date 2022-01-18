# Python3 http.server for Single Page Application

import urllib.parse
import http.server
import socketserver
import re
from pathlib import Path
import os

HOST = ('0.0.0.0', 8080)
pattern = re.compile('.png|.jpg|.jpeg|.js|.css|.ico|.gif|.svg|.json|.csv', re.IGNORECASE)


class Handler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        url_parts = urllib.parse.urlparse(self.path)
        request_file_path = Path(url_parts.path.strip("/"))

        ext = request_file_path.suffix
        if not request_file_path.is_file() and not pattern.match(ext):
            self.path = 'index.html'

        return http.server.SimpleHTTPRequestHandler.do_GET(self)
    def do_POST(self):
        url_parts = urllib.parse.urlparse(self.path)
        request_file_path = Path(url_parts.path.strip("/"))
        params = request_file_path.parts

        if len(params) == 4 and params[0] == "led" and params[1].isnumeric():
            ledNumber = int(params[1])
            if ledNumber >= 0 and ledNumber < 7 and (params[2] == "green" or params[2] == "red") and (params[3] == "on" or params[3] == "off" or params[3] == "blink"):
                command = "sudo ./message-sender.out" + " " + params[0] + " " + params[1] + " " + params[2] + " " + params[3]
                print(command)
                os.system(command)
                self.send_response(200)
            else:
                self.send_response(400)
        elif len(params) == 2 and params[0] == "servo" and params[1].isnumeric():
            angle = int(params[1])
            if angle >= 0 and angle <= 180:
                command = "sudo ./message-sender.out" + " " + params[0] + " " + params[1]
                print(command)
                os.system(command)
                self.send_response(200)
            else:
                self.send_response(400)
        elif len(params) == 1 and params[0] == "start":
            command = "sudo ./message-sender.out" + " " + params[0] 
            print(command)
            os.system(command)
            self.send_response(200)
        else:
            self.send_response(400)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        

if __name__ == "__main__":        
    httpd = socketserver.TCPServer(HOST, Handler)
    print("Server started http://%s:%s" % (HOST))

    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass

    httpd.server_close()
    print("Server stopped.")
