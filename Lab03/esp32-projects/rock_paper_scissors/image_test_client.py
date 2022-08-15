from PIL import Image
import socket


def send_image(s, image):
    bytes_sent = 0
    while bytes_sent < 96*96:
        sent = s.send(image[bytes_sent:])
        bytes_sent = bytes_sent + sent

def client(addr, port):
    while True:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((addr, port))
        image = Image.effect_noise((96,96), 1000)
        image_buf = image.tobytes()
        send_image(s, image_buf)
        s.close()

client("localhost", 8081)