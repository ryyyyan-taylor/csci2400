# Bomb Lab

This lab was our first assignment using GDB and reading Assembly. We had to place the correct breakpoints and read through the assembly in order to figure out what input the "bomb" wanted in order to not blow up and lower our score.

##bomb
This is my bomb; each student received a personalized file upon starting the lab, so the answers would be different in each. When run without breakpoints, if a wrong answer was inputted the bomb would "explode" and lower your total grade. The bomb is able to be run with a text file containing the solutions, so you don't have to remember the solution for each phase or risk typing them in wrong each time.

## bomb.c
This was the file which actually executed the bomb, running each of the phases and such. This was here just for reference, as all the information about each phase is hidden within other header files, forcing you to run the bomb and look at the assembly of each phase.

## bomb-assembly.txt
The assembly code of the bomb. With a combination of reading this and your current point in the bomb's assembly, I was able to figure out what input the bomb wanted.

## bomb-answers.txt
I would run the bomb with this as the argument each time. This contains my correct answers for each phase of the bomb on an independent line.

## phase 5 cypher.txt
Phase 5 took a string in, scrambled the letter individually, and read that scrambled string as the answer. I created this cypher by running this phase with different combinations of letters and noting the outputs.