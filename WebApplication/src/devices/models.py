from django.db import models

# Create your models here.

class Device(models.Model):
    widget_name = models.CharField(max_length=20, default="kitchen")
    name        = models.CharField(max_length=30, default="node1") 
    group       = models.CharField(max_length=30, default="test")
    #We choose what part of microcontroller to trigger
    action      = models.CharField(max_length=3, default="specify pin or choose all")
    description = models.TextField(default="Your short description")
