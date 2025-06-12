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
Screenshot of output meant to represent respective results of
<ul>
  <li>rA, "packed", 2 fields, populated with C++ expr <tt>rA.packField({0,3} ,-1000).packField({4,5} ,7);</tt></li>
  <li>rX, "not packed", no fields & hence 5 distinct bytes, populated with C++ expr <tt>rX.populate(eSign::Negative, 2,4,6,58,9);</tt></li>
</ul>

![emu-demo-0](https://github.com/user-attachments/assets/3d77ca21-7c3f-47fc-a952-cb649005fd57)

(Mark <tt>?</tt> denotes a byte with unknown contents.)

<hr/>
Screenshot of output representing examples of <tt>LDA</tt> op:

![emu-demo-1](https://github.com/user-attachments/assets/1cc9ad9b-66df-4bac-be05-cd5c98520f09)

<hr/>
Screenshot of output representing  examples of <tt>STA</tt> op:

![emu-demo-2](https://github.com/user-attachments/assets/82b08965-fbbb-47bf-aa26-8d26224db525)

<br/>
