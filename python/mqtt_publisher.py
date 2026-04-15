import time, random, math, json
import paho.mqtt.client as mqtt

BROKER = "mqtt"          # Docker service name
GROUP  = "group21"       # change to your group
TOPIC  = f"sensors/{GROUP}/bearing/data"

# client = mqtt.Client()
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.connect(BROKER, 1883, 60)

t = 0
degradation = 0.0       # increases over time to simulate wear

while True:
    # Normal vibration: sinusoidal + noise
    base_vibration = 2.0 + degradation
    noise = random.uniform(-0.3, 0.3)
    vibration = base_vibration + 0.8 * math.sin(t / 5) + noise

    # Occasionally inject a spike (bearing knock)
    if random.random() < 0.04:
        vibration += random.uniform(2.0, 5.0)

    # Slowly increase degradation over time (simulates wear)
    degradation += 0.002

    payload = {
        "vibration": round(vibration, 3),
        "degradation_level": round(degradation, 3),
        "timestamp": time.time()
    }

    client.publish(TOPIC, json.dumps(payload))
    print(payload)
    t += 1
    time.sleep(2)