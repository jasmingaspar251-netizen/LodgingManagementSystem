# Lodging Management System

**OVERVIEW OF THE PROJECT  AND ITS PURPOSE**

The Lodging Management System is a console-based application written in the C programming language that simulates the daily operations of a lodging or hotel business. Its main purpose is to help manage rooms, guest reservations, occupancy status, and waiting guests in an organized and efficient way.

The system allows users to:

View all available and occupied rooms
Check guests in and out
Add new rooms to the system
Search rooms by room type
Sort room records
Manage a guest waitlist when rooms are unavailable

This project demonstrates how core data structures and algorithms can solve real-world business problems such as room allocation, searching, sorting, and queue management.

 **THE DATA STRUCTURES AND ALGORITHMS USED, AND WHICH FEATURES THEY SUPPORT**
 
The system uses several data structures and algorithms to support its features. An array stores all room records, allowing quick access and display of room information. A linked list combined with a queue (FIFO) manages the waitlist, ensuring guests are served in the order they joined. A hash table is used for fast room lookup during check-in, checkout, and room validation.

For algorithms, hashing enables quick room searches, linear search is used to find available rooms by type, and bubble sort organizes rooms by price or room number. Together, these structures and algorithms make the system organized, efficient, and suitable for real-world lodging management operations.

**INSTRUCTIONS ON hHOW TO COMPILE AND RUN THE APPLICATION**

How to Compile and Run Using Dev-C++
1. Open Dev-C++.
2. Click File → New → Source File.
3. Copy and paste your Lodging Management System code into the editor.
4. Click File → Save As and save it as:
    lodging_management.c
5. To compile the program:
    Click Execute → Compile
    Or press F9
6. To compile and run the program directly:
    Click Execute → Compile & Run
    Or press F11
7. After running, the console window will appear showing the Lodging Management System menu, and you can start testing the program by entering your choices.

**Important: Make sure your file is saved with the .c extension so Dev-C++ treats it as a C program.**
