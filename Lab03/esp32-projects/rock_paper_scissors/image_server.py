import numpy as np
import matplotlib.pyplot as plt
from matplotlib import animation

import socket
import threading

image_buf = b''
class_string = []
categories = ['paper', 'rock', 'scissors']


def receive_image(con):
    msg_parts = []
    bytes_recv = 0
    while (bytes_recv < 96*96*3):
        msg_part = con.recv(min(96*96*3 - bytes_recv, 4096))
        msg_parts.append(msg_part)
        bytes_recv = bytes_recv + len(msg_part)
    return b''.join(msg_parts)    

def server(port):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind(("192.168.201.204", port))
    s.listen()
    while True: 
        con, addr = s.accept()
        global class_string
        category_index = int.from_bytes(con.recv(1), byteorder='little')
        score = np.int8(int.from_bytes(con.recv(1), byteorder='little'))
        class_string = "Category: " + str(categories[category_index]) + " Score: " + str(score)
        global image_buf
        image_buf = receive_image(con)
        con.close()

server_thread = threading.Thread(target=server, args=[8081])
server_thread.start()


data = np.zeros((96,96,3), dtype=np.uint8)
fig = plt.figure()
ax = fig.add_subplot(1,1,1)
im = ax.imshow(data, vmin=0, vmax=255, animated=True)
text = ax.text(5, 5, "test", bbox=dict(facecolor='white', alpha=0.5))

def update_image(i):
    if image_buf != b'':
        data = np.frombuffer(image_buf, dtype=np.uint8)
        data = data.reshape((96,96,3))
      #  data = data[:,:,::-1]
        im.set_array(data)
        text.set_text(class_string)
        # time.sleep(.5)
        # plt.pause(0.5)


ani = animation.FuncAnimation(fig, update_image, interval=0)
plt.show()



