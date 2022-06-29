from bluepy import btle
from geometry_msgs.msg import Twist
import rospy
import time
import roslib
import rospy
from kidbright_tpu.msg import tpu_object
from kidbright_tpu.msg import tpu_objects
from std_msgs.msg import String
import rosnode
import subprocess
import time
import os

class MyDelegate(btle.DefaultDelegate):
    def __init__(self):
        btle.DefaultDelegate.__init__(self)

    def handleNotification(self, cHandle, data):
        print(data)
   
# Initialisation  -------
def bleConnect(addr):
    p = btle.Peripheral(addr)   #NodeMCU-32S
    # Setup to turn notifications on, e.g.
    svc = p.getServiceByUUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E")
    p.setDelegate( MyDelegate())
    return svc

#kidbright mac address 
addr = "B4:E6:2D:A5:5F:8B"
svc = bleConnect(addr)
ch_Tx = svc.getCharacteristics("6E400002-B5A3-F393-E0A9-E50E24DCCA9E")[0]
ch_Rx = svc.getCharacteristics("6E400003-B5A3-F393-E0A9-E50E24DCCA9E")[0]

dance = None
e = None

rospy.init_node('get_center', anonymous=True)
velocity_publisher = rospy.Publisher('/cmd_vel', Twist, queue_size=1)
vel_msg = Twist()

ros_nodes = rosnode.get_node_names()
if not '/image_class' in ros_nodes:
	command='rosrun kidbright_tpu tpu_classify.py /home/pi/kbclientNew/nectec-client/dist/Dance_Esan'
	process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
	time.sleep(10)
  
print("wait 1 sec")
time.sleep(1)
try:
  ch_Tx.write(bytes("STA", 'utf-8'), True)
except btle.BTLEException:
  print("ST btle.BTLEException")
  while True:
    try:
      print("trying to reconnect with "  )
      svc = bleConnect(addr)
      ch_Tx = svc.getCharacteristics("6E400002-B5A3-F393-E0A9-E50E24DCCA9E")[0]
      ch_Rx = svc.getCharacteristics("6E400003-B5A3-F393-E0A9-E50E24DCCA9E")[0]
                 
      print("re-connected to ")
      time.sleep(1)
      break
    except:
      continue

while not rospy.is_shutdown():
  data = rospy.wait_for_message('/tpu_objects', tpu_objects, timeout=4).tpu_objects

  for obj in data : 
    print("obj",obj.label)
    if (obj.label == "dance1_KT") :
      bleData = "111"
    elif (obj.label == "dance2_S") :
      bleData = "222"
    elif (obj.label == "dance3_T") :
      bleData = "333"
    elif (obj.label == "background") :
      bleData = "444"
    try:
      print(bleData)
      #send data to kidbright 
      ch_Tx.write(bytes(bleData, 'utf-8'), True)
    except btle.BTLEException:
      print("btle.BTLEException1")
      while True:
        try:
          print("trying to reconnect with "  )
          svc = bleConnect(addr)
          ch_Tx = svc.getCharacteristics("6E400002-B5A3-F393-E0A9-E50E24DCCA9E")[0]
          ch_Rx = svc.getCharacteristics("6E400003-B5A3-F393-E0A9-E50E24DCCA9E")[0]
                      
          print("re-connected to ")
          time.sleep(1)
          break
        except:
          continue
      break  #detect first object then exit loop 
