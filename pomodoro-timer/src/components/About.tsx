import "../styles/about.css";

export default function About() {
  return (
    <main className="about">
      <h1>About</h1>
      <p className="names">Made with love by Anna, Dani, Razan, and Sydney!</p>
      <p>
        “LED Pomodoro Timer” is an embedded study timer designed to aid focus by
        dividing two hour blocks into sections of 'focus' and 'rest.'' This
        technique is based on scientific studies that show time doesn't generate
        stress when it is viewed as a series of events, which is what the
        Pomodoro Technique aims to do. The sequence is as follows: 25 minutes of
        focus time followed by a 5 minute break counts as one focus session, and
        one complete Pomodoro cycle includes 4 focus sessions followed by a
        longer break, typically 15 minutes.
      </p>
      <p>
        This project implements the Pomodoro Technique through a physical
        display, which includes a screen displaying a timer and message
        indicating which stage of the sequence the user is in. Buttons can be
        used to start and pause the timer, and reset the timer. Additionally,
        color-coded LEDs will light up according to the phase of the timer, with
        red indicating focus, green indicating a short break, blue indicating a
        long break, and white for when the timer is idle. There is also a
        website component to this project featuring a dashboard displaying the
        user's focus insights, and a form that can be used to customize the
        duration for each phase of the cycle.
      </p>
      <p>
        Phones and computers can be distracting, therefore a physical device
        that functions purely as a focus timer is ideal for improving focus and
        utilizing time efficiently. Our target audience for the LED Pomodoro
        Timer applies to any students who are having trouble focusing or simply
        desire a better structure to their study time. In general, our project
        is an excellent solution for anyone seeking a distraction-free device to
        add structure to their time.
      </p>
    </main>
  );
}
