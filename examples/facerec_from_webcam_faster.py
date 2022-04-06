import face_recognition
import cv2
import numpy as np
import time

from practicum import find_mcu_boards, McuBoard, PeriBoard
devices = find_mcu_boards()
mcu = McuBoard(devices[0])
peri = PeriBoard(mcu)

# Get a reference to webcam #0 (the default one)
video_capture = cv2.VideoCapture(0)

# Load a sample picture and learn how to recognize it.
obama_image = face_recognition.load_image_file("obama.jpg")
obama_face_encoding = face_recognition.face_encodings(obama_image)[0]

# Load a second sample picture and learn how to recognize it.
biden_image = face_recognition.load_image_file("biden.jpg")
biden_face_encoding = face_recognition.face_encodings(biden_image)[0]

champ_image = face_recognition.load_image_file("champp.jpg")
champ_face_encoding = face_recognition.face_encodings(champ_image)[0]

stamp_image = face_recognition.load_image_file("stamp.jpg")
stamp_face_encoding = face_recognition.face_encodings(stamp_image)[0]


# Create arrays of known face encodings and their names
known_face_encodings = [
    obama_face_encoding,
    biden_face_encoding,
    champ_face_encoding,
    stamp_face_encoding
]
known_face_names = [
    "Barack Obama",
    "Joe Biden",
    "Champ",
    "Stamp"
]

# Initialize some variables
face_locations = []
face_encodings = []
face_names = []
process_this_frame = True

timeout = 150           # 150 sec
start_time = 0


while True:

    try :
        start_tt = mcu.usb_read(4, length=1)

        # 2 == door opened
        if start_tt[0] == 2 :
            print("open door")

        # 1 == check pass
        if start_tt[0] == 1 :
            if time.time() < start_time + timeout:
                tmp = mcu.usb_read(2, length=10)
                
                read_hardware_password = ""
                print(tmp)
                for i in tmp:
                    if str(i) != "0" :
                        read_hardware_password = read_hardware_password + str(i)
                print(read_hardware_password)

                if read_hardware_password == "" :
                    pass
                elif read_hardware_password == "3579" :
                    print("Is TRUE")
                    mcu.usb_write(3, value=1)
                    # break
                else :
                    print("Is False")
                    mcu.usb_write(3, value=0)

            else:
                mcu.usb_write(1, value=0)         # set end        

        # 0 == check face
        if start_tt[0] == 0 :
            # Grab a single frame of video
            ret, frame = video_capture.read()

            # Resize frame of video to 1/4 size for faster face recognition processing
            small_frame = cv2.resize(frame, (0, 0), fx=0.25, fy=0.25)

            # Convert the image from BGR color (which OpenCV uses) to RGB color (which face_recognition uses)
            rgb_small_frame = small_frame[:, :, ::-1]

            # Only process every other frame of video to save time
            if process_this_frame:
                # Find all the faces and face encodings in the current frame of video
                face_locations = face_recognition.face_locations(rgb_small_frame)
                face_encodings = face_recognition.face_encodings(rgb_small_frame, face_locations)

                face_names = []
                for face_encoding in face_encodings:
                    # See if the face is a match for the known face(s)
                    matches = face_recognition.compare_faces(known_face_encodings, face_encoding)
                    name = "Unknown"

                    # Or instead, use the known face with the smallest distance to the new face
                    face_distances = face_recognition.face_distance(known_face_encodings, face_encoding)
                    best_match_index = np.argmin(face_distances)
                    if matches[best_match_index]:
                        name = known_face_names[best_match_index]
                    face_names.append(name)
                
                if("Champ" in face_names):
                    # mcu.usb_write(0, value=1, index=0)
                    # mcu.usb_write(3, value=90)
                    # peri.set_led(1,1)

                    mcu.usb_write(1, value=1)         # set start
                    
                    start_time = time.time()
                    
                print(face_names)
            process_this_frame = not process_this_frame

    except :
        print("ERROR !!!!!!!!!!!!!!!!!!!!!!")
        devices = find_mcu_boards()
        mcu = McuBoard(devices[0])
        peri = PeriBoard(mcu)
    
    time.sleep(1)       # sleep 1 sec
    for (top, right, bottom, left), name in zip(face_locations, face_names):
        # Scale back up face locations since the frame we detected in was scaled to 1/4 size
        top *= 4
        right *= 4
        bottom *= 4
        left *= 4

        # Draw a box around the face
        cv2.rectangle(frame, (left, top), (right, bottom), (0, 0, 255), 2)

        # Draw a label with a name below the face
        cv2.rectangle(frame, (left, bottom - 35), (right, bottom), (0, 0, 255), cv2.FILLED)
        font = cv2.FONT_HERSHEY_DUPLEX
        cv2.putText(frame, name, (left + 6, bottom - 6), font, 1.0, (255, 255, 255), 1)

    # Display the resulting image
    cv2.imshow('Video', frame)

    # Hit 'q' on the keyboard to quit!
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

    


# Release handle to the webcam
video_capture.release()
cv2.destroyAllWindows()