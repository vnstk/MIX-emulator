# MIX-emulator
Emulator (extremely incomplete) of the "thought experiment" MIX computer from TAOCP ed3 vol1.

MIX is not easy to emulate, because
<ul>
  <li>A word can be divided into fields of arbitrary size.</li>
  <li>The sign can be represented by its own field, be a part of a field, or pertain to all of a word's bytes.</li>
  <li>Some legal operations result in fields with an indeterminate value.</li>
  <li>A byte can hold at least 64 different values (so, 6 bits); but maybe more.</li>
</ul>

Below, screenshot of output, meant to represent respective results of
<ul>
  <li>rA, "packed", 2 fields, populated with C++ expr <tt>rA.packField({0,3} ,-1000).packField({4,5} ,7);</tt></li>
  <li>rX, "not packed", no fields & hence 5 distinct bytes, populated with C++ expr <tt>rX.populate(eSign::Negative, 2,4,6,58,9);</tt></li>
  <li>Examples of <tt>LDA</tt> instruction from p129.</li>
</ul>

![emuMIX-basics-screensh](https://github.com/user-attachments/assets/49b5b2f1-af62-47e8-b87b-4a3bca80e5e4)

(Mark <tt>?</tt> denotes a byte with unknown contents.)
