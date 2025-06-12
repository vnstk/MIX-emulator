# MIX-emulator
Emulator (extremely incomplete!!) of the "thought experiment" MIX computer from TAOCP ed3 vol1.

MIX is not easy to emulate, because
<ul>
  <li>A word can be divided into fields of arbitrary size.</li>
  <li>The sign can be represented by its own field, be a part of a field, or pertain to all of a word's bytes.</li>
  <li>Some legal operations result in fields with an indeterminate value.</li>
  <li>A byte can hold at least 64 different values (so, 6 bits); but maybe more.</li>
</ul>

<hr/>
Screenshot representing, in top line,
<ul>
  <li>rA, "packed", 2 fields, populated with C++ expr <tt>rA.packField({0,3} ,-1000).packField({4,5} ,7);</tt></li>
  <li>rX, "not packed", no fields & hence 5 distinct bytes, populated with C++ expr <tt>rX.populate(eSign::Negative, 2,4,6,58,9);</tt></li>
</ul>

![emu-demo-0](https://github.com/user-attachments/assets/99b0b96d-6527-4dbf-a90c-2fa73be84453)

Middle line is rA, "packed", 3 fields of which the 1st is just the sign & the other two are "real" data fields. Neither data field "owns" the  sign.  In this case, rA was populated with C++ expr <tt>packField({1,3} ,1000).packField({4,5} ,7);</tt> --- sign is positive because that's the default per text.

Bottom line is Middle line is rA, "packed", this time via C++ expr <tt>rA.packField({0,0} ,-1).packField({1,3} ,1000).packField({4,5} ,7);</tt>.  Sign is negative because we explicitly specified its "field".

(In the screenshot, overlaid arrow points to the vertical bar which denotes that critical detail: if no bar separates sign and Lmost numeric value, then sign "is part of" the first "real data" field.)

So, here we have all the cases:
<ul>
  <li>sign is "on its own": rA in middle line, rA in bottom line.</li>
  <li>sign "is part of" a field: rA in top line.</li>
  <li>sign pertains to all of the word's bytes: rX in top line.</li>
</ul>

<hr/>
Screenshot of output representing examples of <tt>LDA</tt> op:

![emu-demo-1](https://github.com/user-attachments/assets/1cc9ad9b-66df-4bac-be05-cd5c98520f09)

(Mark <tt>?</tt> denotes a byte with indeterminate contents.)

<hr/>
Screenshot of output representing  examples of <tt>STA</tt> op:

![emu-demo-2](https://github.com/user-attachments/assets/82b08965-fbbb-47bf-aa26-8d26224db525)

<br/>
