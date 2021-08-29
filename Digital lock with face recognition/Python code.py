import cv2
from cv2 import *
import face_recognition
import serial
import sys

ser = serial.Serial('COM4', baudrate=9600, timeout=1)

cascPath = sys.argv[0]
faceCascade = cv2.CascadeClassifier(r"E:\python programs\Embd with python\Basics\Face-Detection-Recognition-Using-OpenCV-in-Python-master\haarcascade_frontalface_default.xml")
video_capture = cv2.VideoCapture(0)

classifier = CascadeClassifier(r"E:\python programs\Embd with python\Basics\Face-Detection-Recognition-Using-OpenCV-in-Python-master\haarcascade_frontalface_default.xml")
known_image_1 = face_recognition.load_image_file(r"C:\Users\hp\Pictures\T1.jpg")

while True:
    ret, frame = video_capture.read()
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    faces = faceCascade.detectMultiScale(
        gray,
        scaleFactor=1.1,
        minNeighbors=5,
        minSize=(30, 30),
    )
    for (x, y, w, h) in faces:
        cv2.rectangle(frame, (x, y), (x+w, y+h), (0, 255, 0), 2)

    cv2.imshow('Video', frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

count = 0
for (x, y, w, h) in faces:
    face = frame[y:y+h, x:x+w]  # slice the face from the image
    cv2.imwrite(r'C:\Users\hp\Pictures\Captured Image\Image' +str(count) + r'.jpg', face)
    count += 1
    cv2.rectangle(frame, (x, y), (x+w, y+h), (255, 0, 0), 2)

loc = str(r'C:\Users\hp\Pictures\Captured Image\Image' +str(count-1) + r'.jpg')
unknown_image = face_recognition.load_image_file(str(loc))
pixels = cv2.imread(str(loc))

known_image_1_encoding = face_recognition.face_encodings(known_image_1)[0]

known_encodings = [known_image_1_encoding]
unknown_encoding = face_recognition.face_encodings(unknown_image)[0]

results = face_recognition.compare_faces(known_encodings, unknown_encoding)

bboxes = classifier.detectMultiScale(pixels)
for box in bboxes:
    x, y, width, height = box
    x2, y2 = x + width, y + height
    cv2.rectangle(pixels, (x, y), (x2, y2), (0, 0, 255), 1)

if results[0] == True:
    data = "ON"
    ser.write(data.encode())
    print(data)
else:
    print("Wrong Person")

imshow("Face we detected", pixels)
waitKey(0)
destroyAllWindows()
