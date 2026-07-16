# Alarm Clock Build Journal

## Jul 16, 2026, 5:34 AM

I figured out how to use the design system, and I added many components for my alarm clock. The LEDs will light up when the alarm goes off, the speaker will play sounds recored by the microphone, the potentiometer will be used to adjust volumes, and the IR remote will be used to remotely control the alarm clock.

---

## Jul 16, 2026, 2:53 PM

I started coding and figured out how to use some of the components such as the lcd and the keypad. I decided to use a state machine because it would be easy to implement all the functionality that I want, with some things happening simultaneously. I implemented the functionality of displaying the time on the lcd, and recording presses from the keypad.

---

## Jul 16, 2026, 4:02 PM

I have a pretty complex state system now, but I think that it will be effective for making the ux feel good. It's interesting to make something like this from scratch, it shows you just how much operating systems are doing in the background.

---

## Jul 16, 2026, 5:36 PM

Since the project is getting much larger than I anticipated, I think I'm going to change the structure a little bit.

---

## Jul 16, 2026, 8:41 PM

I changed the project a lot. I changed the state system so that now each state has two functions, which are init and run, which are like setup and loop. It is also using an enum now since I realized that you can't use switch on just anything.

---

## Jul 16, 2026, 11:04 PM

The code is starting to get pretty repetitive, but I have almost finished all of the functionality. I add most of the states, which were mostly just updating data and making it interactive on the lcd. Now I am working on the alarm state, and it should be finished.

---

## Jul 16, 2026, 11:21 PM

I'm just working on finishing touches, mostly making sure that the volume pop works properly with the timeouts