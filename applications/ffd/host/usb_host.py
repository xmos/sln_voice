import usb.core
import usb.util

from time import sleep

OP_CODE_GET_LAST_INTENT_HEX = "01"
KEYWORD_ENDPOINT = 0x82

# find our device
dev = usb.core.find(idVendor=0x20B1, idProduct=0x0020)

if dev is None:
    raise ValueError('XCORE-VOICE device not found')

# print(dev)

dev.reset()
dev.set_configuration()
dev.reset()

intent_to_text = {
    100 : "Hello XMOS",
    110 : "Hello Wanson",
    200 : "Switch On the TV",
    210 : "Switch Off the TV",
    220 : "Channel Up",
    230 : "Channel Down",
    240 : "Volume Up",
    250 : "Volume Down",
    300 : "Switch On the Lights",
    310 : "Switch Off the Lights",
    320 : "Brightness Up",
    330 : "Brightness Down",
    400 : "Switch On the Fan",
    410 : "Switch Off the Fan",
    420 : "Speed Up the Fan",
    430 : "Slow Down the Fan",
    440 : "Set Higher Temperature",
    450 : "Set Lower Temperature"
}

while True:
    sleep(0.1)
    try:
        dev.write(0x02, bytes.fromhex(OP_CODE_GET_LAST_INTENT_HEX))
        resp = dev.read(KEYWORD_ENDPOINT, 4)
        if resp:
            int_val = int.from_bytes(resp, "little")

            if int_val in intent_to_text.keys():
                print(intent_to_text[int_val])
    except:
        continue
