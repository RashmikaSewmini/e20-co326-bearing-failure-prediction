import json, time
import numpy as np
import paho.mqtt.client as mqtt

BROKER     = "mqtt"
GROUP      = "group21"
SUB_TOPIC  = f"sensors/{GROUP}/bearing/data"
ALERT_TOPIC = f"alerts/{GROUP}/bearing/status"

# Rolling window for baseline
window = []
WINDOW_SIZE = 20

def detect_anomaly(value, history):
    if len(history) < WINDOW_SIZE:
        return False, "collecting"
    mean = np.mean(history)
    std  = np.std(history)
    # Z-score anomaly: more than 2.5 std deviations from mean
    z_score = (value - mean) / (std + 1e-6)
    if value > 7.0:
        return True, "CRITICAL - bearing failure imminent"
    elif value > 5.0 or z_score > 2.5:
        return True, "WARNING - abnormal vibration"
    return False, "NORMAL"

def on_message(client, userdata, msg):
    data = json.loads(msg.payload)
    vib  = data["vibration"]
    window.append(vib)
    if len(window) > WINDOW_SIZE:
        window.pop(0)

    anomaly, status = detect_anomaly(vib, window)

    alert = {
        "status": status,
        "vibration": vib,
        "anomaly": anomaly,
        "timestamp": time.time()
    }
    client.publish(ALERT_TOPIC, json.dumps(alert))
    print(f"[AI] {status} | vib={vib:.2f}")

# client = mqtt.Client()
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.on_message = on_message
client.connect(BROKER, 1883, 60)
client.subscribe(SUB_TOPIC)
client.loop_forever()