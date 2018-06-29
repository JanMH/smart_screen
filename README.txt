About
=====

This project is a non-intrusive monitoring prototype written in C++. It performs transient state analysis on electrical currents. Events are detected by comparing the root mean square of specific time frames and then classsifying these events with knn classification based on a model of known events.


For build instructions please refer to the file INSTALL.txt


Running the Program
===================
To run the program, navigate into the folder "<project dir>/working_directory", copy and modify the file "config.ini.example" to your liking and finally execute:

../build/nofare_app/nofare_app config.ini


Extending the Program
=====================

In the branch nofare-testapp is a subdirectory "nofare_app". It is already set up to be extended by adding code the the file "<project dir>/nofare_app/src/server/InputSource.cpp". The function readDataPoints must read data from the usb interface and insert it into the async data queue. The rest of the project is already set up to analyze the incoming data for events and generate notifications to the standard output. The event handlers are located in the directory "<project dir>/nofare_app/src/server/EventHandlers.cpp". 

Disclaimer
=========

This is a project taken from my work for the chair of business information systems at the TU München. Some parts were removed to not expose private API functions. The code was not extensively tested without these parts and may crash or worse. Please proceed with caution.

