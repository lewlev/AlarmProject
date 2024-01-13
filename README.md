# Alarm Project
## Part I Alarm Controller
**Project objective**

To create a home security monitor/alarm using elements of a pre-existing but redundant alarm system. The Alarm Controller should also be capable of triggering a separate security camera wirelessly. The camera is to be part II of this project.

**Background**

With the demise of old PSTN landline phone system, some older home security systems that relied on that PSTN network to access remote monitoring became redundant. There are several ways to address that situation, but for fun it seemed to me that it was a good candidate for some kind of IoT connected micro controller. Another reason you might go down this path is if you have an existing security system that had  a free internet based monitoring, but  that has now changed to a subscription model.

**System design requirements:**
* Be powered entirely from the exiting alarm system power supply.
* Use a mobile phone for all control and monitoring from anywhere.
* Connect to an internet based IoT service for monitoring, control and communication with other IoT devices.
* Monitor up to four sensors and raise an alarm if a  breach of security is detected.
* In the event of an alarm, respond in any or all of the following actions:
*    *Sound an alarm.
*    *Flash a strobe light.
*    *Send an alert email
*    *Send a push notification to a mobile phone
* Be compliant with relevant regulations regarding limitations to duration of siren on time etc.
* Behave in a predictable and compliant way in the event that the  internet connection is lost, or a dependent internet service fails.
* Have the ability to connect wirelessly with other IoT devices to add functionality.
* Be capable of receiving firmware updates via Wi-Fi, known as  over the air (OTA)
* Be powered from the exiting alarm system via PIR wiring.
* Use a mobile phone for all control and monitoring from anywhere.
* Connect to an internet based IoT service for monitoring, control, and communication with other IoT Devices.
* To capture and store videos or still images when triggered by an alarm.
* To upload all images/videos to cloud .
* Be capable of receiving firmware updates via Wi-Fi, known as  over the air (OTA).



For the full project description have a look at:

https://github.com/lewlev/AlarmProject/blob/main/AlarmProjectDescriptionV7.pdf




The full source and documentation is in Pack2.zip here

https://github.com/lewlev/AlarmProject/blob/main/Pack1.zip

Download and extract to c:\AlarmProject