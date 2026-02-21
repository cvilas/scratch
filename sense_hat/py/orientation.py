from sense_hat import SenseHat
sense = SenseHat()
while True:
    pitch, roll, yaw  = sense.get_orientation().values()
    print(f"Pitch={round(pitch,1)}, Roll={round(roll,1)}, Yaw={round(yaw,1)}")

