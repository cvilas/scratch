from sense_hat import SenseHat
sense = SenseHat()
while True:
    ax, ay, az  = sense.get_accelerometer_raw().values()
    print(f"ax={round(ax,1)}, ay={round(ay,1)}, az={round(az,1)}")


