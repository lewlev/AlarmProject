Project objective

To create a home security monitor/alarm using elements of a pre-existing but redundant alarm system.

Background

With the demise of old PSTN landline phone system, some older home security systems that relied on that PSTN network to access remote monitoring became redundant. There are several ways to address that situation, but for fun it seemed to me that it was a good candidate for some kind of IoT connected micro controller. Another reason you might go down this path is if you have an existing security system that had  a free internet connected monitoring system that is now  changed to a subscription model.


System requirements:

●	Be powered entirely from the exiting alarm system power supply.
●	Use a mobile phone for all control and monitoring from any where.
●	Connect to an internet based IoT service for monitoring, control and communication with other IoT devices.
●	Monitor up to four sensors and raise an alarm if a  breach of security is detected.
●	In the event of an alarm, respond in any or all of the following actions:
 ○	Sound an alarm
 ○	Flash a strobe light.
 ○	Send an alert email.
 ○	Send a push notification to a mobile phone.
●	Be compliant with relevant regulations regarding limitations to duration of siren on time etc.
●	Behave in a predictable and compliant way in the event that the  internet connection is lost, or a dependent internet service fails.
●	Have the ability to connect wirelessly with other IoT devices to add functionality.
●	Be capable of receiving firmware updates via Wi-Fi, known as  over the air (OTA).


Finished project operation . 
 The alarm system is very simple to operate, it can be controlled and monitored with a mobile phone  from anywhere as long as there is coverage and internet. There are only two user inputs and seven status indicators. No codes are required to access the app or the alarm panel.


Connecting up the system

More details including construction and programming can be found in 'AlarmProjectDescriptionV5.pdf'



The file Pack.Zip is included with this project includes all software source code files, templates for IOT Thing and Dashboards, higher res circuit and layouts some more detailed set up guides for IFTTT etc. First thing to do if you are interested in building this is to create a folder in the root of the C drive C:\AlarmProject. ( you may need to be granted  elevated privileges to do this)    Click Pack.zip  to download the file then extract it to into that folder.