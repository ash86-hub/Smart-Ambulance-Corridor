# Smart-Ambulance-Corridor
Smart Ambulance Corridor is a Grade 7 Arduino project that shows how traffic lights can give ambulances priority. An SOS button clears the route to the patient. After pickup, the signals clear the route to the hospital, and an LED represents the hospital alert. A future version could use GPS and a mobile app.

Our proposed solution is a Smart Ambulance Corridor that uses GPS and coordinated traffic signals to help ambulances reach patients and hospitals with fewer delays.

When someone sends an SOS through a mobile app, the control centre receives the patient’s location and alerts the nearest available, suitable ambulance. As the ambulance approaches each junction, the traffic controller gives its direction a green light after conflicting traffic has cleared. The signal returns to its normal cycle once the ambulance passes.

After pickup, the crew selects a suitable hospital. The hospital receives an alert so staff can prepare, and the system coordinates the signals along the new route. If two ambulances need conflicting routes through the same junction, the controller gives them priority one at a time, taking medical urgency and estimated arrival times into account.

Our Arduino model demonstrates this idea across four junctions using SOS buttons, preset routes and LED signals. When two SOS requests are made, the second waits until the first journey is complete.
