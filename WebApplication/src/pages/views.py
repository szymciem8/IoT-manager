from django.shortcuts import render
from django.http import HttpResponse

from devices.models import Device

import paho.mqtt.client as mqtt

#MQTT 

mqtt_topic = "test"
mqtt_broker_ip = "192.168.0.104"

client = mqtt.Client("python")

def on_connect(client, userdata, flags, rc):
    print("Connected!", str(rc))
    client.subscribe(mqtt_topic)


# Create your views here.

def index_view(request):
    
    object_context = []

    number = Device.objects.all().count()
    for i in range(number):
        print(i)
        obj = Device.objects.get(id=i+1)
        temp_dict = {'widget_name':obj.widget_name,
                    'description':obj.description}

        object_context.append(dict(temp_dict))

    context = {
        'widget_list': object_context
    }

    return render(request, "index.html", context)

def home_view(request):
    return render(request, "home.html", {})

def informations_view(request):
    return render(request, "informations.html", {})

def new_device_view(request):
    return render(request, "new_device.html", {})


#MQTT functions
def turn(request):
    widget_id = request.POST.get("id")
    action = request.POST.get("action")

    obj = Device.objects.get(id=int(widget_id))
    context = {
        'name':obj.name,
        'group':obj.group,
        'action':obj.action,
    }
    command = obj.name + " write " + obj.action + " " + action 
    print("command: " + command)
    client.publish(obj.group, command)
    return HttpResponse("""<html><script>window.location.replace('/index');</script></html>""")


client.on_connect = on_connect
client.connect(mqtt_broker_ip, 1883)

#MQTT thread 
client.loop_start()