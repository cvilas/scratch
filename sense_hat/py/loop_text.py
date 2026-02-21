from sense_hat import SenseHat
sense = SenseHat()
while True:   
    msg = f"I am Steve"
    sense.show_message(msg, scroll_speed=0.1, text_colour=[255,255,0], back_colour=[0,0,255])