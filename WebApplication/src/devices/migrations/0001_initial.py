# Generated by Django 3.1.5 on 2021-01-25 11:52

from django.db import migrations, models


class Migration(migrations.Migration):

    initial = True

    dependencies = [
    ]

    operations = [
        migrations.CreateModel(
            name='Device',
            fields=[
                ('id', models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('name', models.CharField(default='node1', max_length=30)),
                ('group', models.CharField(default='test', max_length=30)),
                ('action', models.CharField(default='specify pin or choose all', max_length=3)),
                ('description', models.TextField(default='Your short description')),
            ],
        ),
    ]
