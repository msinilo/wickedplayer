doc for mod player (in fact it's my new sound system)
inspired by technomancer's documentation
-------------------------------------------------------------------------------

project begin 27-06-97 02:58:27
              only some .h files, things like that
              hmmm it's 03:54:37am. maybe i should take a rest?
              oh, yesterday i had my last university entrance exam.
              now i have to wait till the 1st of july -- the results.

28-06-97 21:53:05
huh, some cool things have appeared. drivers/loaders could be easily added.
mod loader works almost perfectly, new gus routines.
player works, although it supports only 3 effects: set volume, set speed
and volume slide. i have some problems with volume ramping, doesn't work...
it seems to doing nothing when no ramp rate is set, and when i set rate
then i can hear nothing. anyway, who cares, simple "set volume" works good.
huh, for the present i have no problems with debug switches, everything works
without it (my previous player didn't run w/o -d2 compiler and debug all
linker switch).

30-06-97 12:10:33
new effects supported: panning, sample offset (finally it seems to work
nearly perfectly), jump to pattern, pattern break, fine portas, extended
panning, fine volslides.
finetune to c2spd table was a bit modified, now it sounds better.
elysium.mod by jester/sanity is replayed very well.
huh, the same goes for rage.mod, also by jester. i like jester. he uses
only volslide + set vol :) (at least in mods i got [rage^elysium]).
tomorrow -- results of my exam...
hmmm, i'm thinking about changing my volume table, it's a bit too loud...

02-07-97 13:42:32
hahaha. i got my exams. and now i have the holidays. cool, not?
my new volume table is even worse than the previous one. not so loud, but
sample-offset clicks are more hearable (don't know if there's such a word
in english :). huh, i think it's time to make some more difficult effects
(arpeggio, vibrato, portamentos and so). so, back to work.

03-07-97 14:01:08
troubles with porta2note. i'm going to code it all from a scratch.
for the present i have: loader (99% bug-free i think) and gus driver
(still without volume ramping; to be honest i must admit i didn't even
tried to make it work). hey, time is money, so, let's stop writing this
crap and go to code. um, on sunday i'm going to mazury with amnesty guys
(brach, chester, warlock). koool. btw. it seems this is my 4th(!) version
of module player (2nd was quite nice, but it was also terrible messy).
15:15:31
player works. no effects. precalculated frequency table for GUS, new period
tables... ufff, hope now it will work.
15:42:46
hehe, elysium and rage are replayed _PERFECTLY_. volume slides works.
btw: i'm not sure if it's allowed to slide up & down in the same moment,
probably not (and that's what some docs say), so it's not supported.
16:24:21
problems with myo.mod. at the beginning of ord 5 it starts being played wrong.
i dunno why? anyway, sample offset works well :)
17:47:11
yeah, volume ramping works! simple as pi :). myo.mod is still replayed wrong.
but volramping it's something really powerful. modules sound much better now.
20:07:24
vibrato almost works
arpeggio probably works
portamento to note works in 40%
journey.mod by some1 and morrow is replayed well. even with 40% of porta2note :)
22:45:20
vibrato works in 90%
arpeggio probably works
portamento to note works in 90%
3d_demo.mod by mad freak/anarchy is replayed well. some problems with
wizardry. now it's time to check myo! uff. i'm back. know where was the bug?
in sample offset of course! it must be zeroed every row and i forgot about it.
sample offset works well NOW! (i hope :). ok, now we have only some quite
easy effects like E-effects and so to implement. back 2 work.
23:29:30
i'll try to add support for 32ch modules.
23:48:20
support added. simpler than pi :). i had only to add new modtype detection
and increase MAX_CHANNELS constant from 8 to 32 and that's all. tritons rulez.
also a bit of interface was added, just for fun, not as a real player-interface.
i do not support tremolo. who uses tremolo? vibrato works only with sine
waveform, i think it covers 90% of vibratos.


04-07-97 01:31:16am
extended octaves (0+4) added. now xxCH mods are replayed better, they use
these octaves quite often.
01:46:17am
sum really hardcore modules by keyG checked (little suxx, conversation).
guess what? wicked player plays them almost as well as capamod does. yeah.
huh, falcon's "pink panther" is real killer. still can't play it rite :(
let's switch on some cool watcom optimize options.
it works. so, go 2 bed :) oh, here is the list of currently supported efx:
0x0 - arpeggio (90%)
0x1 - porta up
0x2 - porta down
0x3 - porta to note
0x4 - vibrato (only sine waveform)
0x5 - porta to note+volume slide
0x6 - vibrato+volume slide
0x7 - tremolo (don't know if it works, coz i don't have any modules using
               tremolo)
0x8 - DMP panning (does anybody use this?)
0x9 - sample offset (finally it works!)
0xA - volslide
0xB - position jump
0xC - set volume
0xD - pattern break
0xE1 - fine slide up
0xE2 - fine slide down
0xE5 - set finetune
0xE9 - retrigger sample (not tested)
0xEA - fine vol slide up
0xEB - fine vol slide down
0xEC - cut sample (not tested)
0xF - set speed

03:20:54am
funny feature added: recognizing polish composers. of course not all of 'em.

13:42:33
hmmm, vibrato seems to work bad :(. dunno why? i will try full sine wave
(2pi not only pi).

05-07-97 14:35:08
for the present i can say that only vibrato and arpeggio (and maybe tremolo)
are bugged. rest works nice. all modules w/o ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
are replayed well. my fave one to test at the moment is journey.mod, i have
just tried it and it was all ok. oh, i will try to improve my vol ramping,
maybe it's a bit too fast (63 for the present; i will try 31), and maybe
i'll also try to do sumthin' like:
a) set the volume to 0
b) fast ramp to selected volume

15:11:05
woooooow. far_away by extend sounds superb with my player. volume ramping
with rate 31 sounds a bit better than 63, at least in my opinion. now it's
time to experiment with different volume tables, rite?

15:30:30
smoke.mod -- next module which sounds just as good with wicked player as
with capamod or cubic player.

06-07-97 12:43:18
f**k. sumthin' wrong is with tone portamento. journey.mod is now replayed
wrong! i dunno why?
12:49:34
haha, you will never guess why there was an error! it had nothing to do
with portamento, it was a bug in my GUSPlayVoice routine! I set wrong
voice mode after finished sending addresses to ports. now it's ok.
the old bug with vibrato was in fact bug with portamento to note...
sometimes, the porta speed is greater than current period, which is greater
than destination period. so, after the first sub our period is smaller than
zero. but my period value was unsigned, so there was an overflow. now it's
all fixed by one small "(word)" prefix. vibrato probably works ok.

14:02:44
geeeeeeeeeeezus. i'm lamer. this time you will _REALLY_NEVER_ guess where
was the bug! i had some problems with almost every chiptune, especially
with wizardry.mod. i was suspecting that it's vibrato bug. the real bug
lied in preparing looped sample to play. of course it should be sumthin'
like:
 GUSPlayVoice(voice, mode, start, loop_start, loop_end);
                                              ---------
and I had it:
 GUSPlayVoice(voice, mode, start, loop_start, len);
                                              ------> sample length
huh, now wizardry is replayed cooooool! the same goes for falcon's vacation_.
now it's a trial time. let's try some more cool chips...

14:12:20
yeah, all cool snoopy's synths are played ok, falcon's pink panther also,
keyG's chips, too. it seems my player is almost finished. now only some
speed optimizations: look-up table for GUS frequencies and faster loading
samples. so, it took me about 10 days to create a module player. but it's
not my first version of coz.

15:43:52
simple bars added. nothing very funktional, but loox kuite kuul (did you
notice my cool elitish style of riting? :)

--------------------------------------------------------------------------
08-07-97 14:24:28
--------------------------------------------------------------------------
yeah! after some bloody days (^nites) i made my s3m player work. well, in
fact the main problem was the loader, not a player itself. s3m is really
strange format. i had less problems with amiga mod than with pc s3m.
of course it's not implemented even in 50%, only some mod effects are
supported, no native s3m efx for the present. loader is very messy and
probably buggy but worx ok with 2nd_pm.s3m... player plays it well, also.
now my mod player is integrated with s3m one... no more finetunes, yeah!
ooook. now i have to fix s3m loader bug: i can't load the last sample...
dunno why...

14:36:00
hehehe. as always - problems with signed/unsigned. my MemSeg was word,
which is -32768..+32767, so, after <<4 we could get value up to 524272,
and there are modules much bigger than that. all i had to do was changing
the type of MemSeg to uword (0..65535), which allows me to load modules
up to 1048576 (and my gus won't accept bigger ones anyway).
why can't you remember me... by marvel/fc is replayed very well, coz it
doesn't use any s3m effects, only set speed.

23:12:45
some problems with strange s3m volume command (D??). in 2nd_pm.s3m
(second reality soundtrack by purple motion) my slides are longer than
e.g. capamod's slides. i DO use fast volume slides, so it's not a reason;
so what? i do not know :(
anyway, i'm happy enough because of working player, so let's don't care
'bout vslides, rite?
oh, there are also problems with calculating the number of channels in s3m
file. but it seems to be problem for every player, since every of 'em
displays different numbers.

09-07-97 11:42:06
da current list of supported effects:
0x0 - arpeggio (a bit fucked with S3Ms!)
0x1 - portamento up
0x2 - portamento down
0x3 - portamento to note
0x4 - vibrato
0x5 - portamento to note+volume slide
0x6 - vibrato+volume slide
0x7 - tremolo (i'm not sure of it)
0x8 - dmp panning (non standard efx)
0x9 - sample offset
0xa - volume slide
0xb - position jump
0xc - set volume
0xd - pattern break
0xe1 - fine porta up
0xe2 - fine porta down
0xe5 - set finetune
0xe8 - panning
0xe9 - retrigger note
0xea - fine vol up
0xeb - fine vol down
0xec - cut note
0xee - pattern delay
0xf - set speed
0x10 - S3M set speed
0x11 - S3M volume slides (i'm not sure of it)
0x12 - S3M slides (doesn't work perfectly)

so, in fact i support almost every single protracker effect except for
waveforms (only sine), glissando, delay note & invert loop (well, invert
loop is not supported by any player). not too bad i think.

 9-07-97 15:49:43
uff. i fixed some bugs in loader/converter/player. now it's all ok! i hope..

sum facts 'bout the player.
) all code in WATCOM C (only few lines in assembly -- gus routines)
) source code is about 4000 lines long
) it uses my shutUp() system -- simple routine which will dehook, stop,
  dealloc and de** all things you have hooked, started etc. and then exit
  to dos. very useful...
) what have i used during writing this player:
  - fmoddoc2 by firelight - documentation on mods/s3ms and much more
  - technomancer's player (volume ramping)
  - modfil10.txt by thunder - great text about mods
  - mod-form.txt by lars hamre and others - all you wanna know about mods
  - tech.doc by (probably) psi/fc - s3m format
  - st3.doc by edge/emf - s3m effects
  - gusmod 2.11 sources by cyberstrike/renaissance - tips & tricks
  - extravaganza module player by mckriss^mr.baggins/exa - "volatiles"
  - cubic player+capa mod - checking if wicked player replays modules correctly
  - samael's letters to me - explanations about gus/s3ms etc.
  - mikmak.h file - i got it 2gether with watcom, but only this file, nuffin'
    more, if someone has got complete player, please, send it 2 me!

10-07-97 00:31:25
added fine vibrato, global volume & retrig+volslide support.

17-07-97 15:12:35
modifications in loaders & drivers, now it's faaaar more stable than it was
before. some hints:
) if player hangs up before playing it's probably caused by MP_CalcSongTime
  routine. i have no strength to fix it now, it's not very important anyway
  and hangs only with few songs.
) i'm searching for xm documentations (i only have xm.txt by mr.h of triton,
  kinda kuul, but some more docs is welcome!).
) well, da drivers system is not very universal, i guess. it's caused by the
  fact i do not have sb, and i even don't know shit about it, so driver
  system is heavily based on gravis ultrasound's features.

17-07-97 18:28:18
bug in arpeggio fixed, finally it works with pieceatt.s3m by falcon, now
it's even better than the one in fmod (it has the same bug as my player had).

some words 'bout using wicked player as the soundsystem, maybe some1 will use
it some day... hoo knowz?

1. initializing drivers^loaders
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
in current version only GUS is supported as a sounddevice, so there's
no problem with detecting etc. all you have to do is:
UsedDrv = &DrvGUS
UsedDrv->Init();        /* of course you should also check returned value,
                           if it's 0 -- GUS not detected or memory error! */

handling loaders is only a bit more difficult. in current version we have
s3m and mod loaders. every loader has pointer to the next one, s3m loader
is first and it points on mod loader, mod loader points to ???, since there's
no more loaders. let's see at this piece of code, its task is to choose
valid loader for given kind of file:

  UsedLdr = &Load_S3M;
  while (UsedLdr)
  {
    if (!UsedLdr->Identify(argv[0]))
      UsedLdr= UsedLdr->Next;
    else
      break;
  }

  if (!UsedLdr)
   /* report error: uknown file format */

of course, if you are sure that the module you're going to play is in s3m
(or mod) format (well, in fact you gotta know what kind of module you
are using in your demo, rite?) you can as well code it in this way:
UsedLdr = &Load_S3M or UsedLdr = &Load_MOD -- simple, huh?

overall: UsedLdr^UsedDrv has to be filled with valid pointers, how will you
do this is up to you.

2. playing module
~~~~~~~~~~~~~~~~~
if there's sumthin' simpler than initializing wicked player than it must
be playing modules with it. all you have to do is call one cool routine:
MP_PlayMod();
!!WARNING!! in current version it takes no arguments, it uses mod which
has just been loaded, but i'm going to change it a bit in future versions.
it will get the argument - pointer to the module structure and will use
only this pointer. what does it mean? you can easily add your own loaders
or module format (well, it can't be more advanced than s3m).

3. stopping module
~~~~~~~~~~~~~~~~~~
you're bored? the muzax suxx?
MP_StopMod will make you free. oh, don't you think you should cleanup now?
UsedLdr->Cleanup() will deallocate all allocated shits.

4. more
~~~~~~~
global variables:
ord - current order
row - current row (0-63)
bpm - current bpm (beats per minute, ticks = bpm*2/5)
speed - current speed
tracks - contains all informations about _every_ played row/channel.
use it as you want (e.g. for volume bars)

!!WARNING!! these variables above are read-only! do not try to change 'em
in any way or it'll hurt.

quite good example of minimal player is wicked.old. wicked.c is a small
player with inertia-like interface, it was coded just4fun. nevermind.........

 2-08-97 03:20:07. yeah, today (oupps, yesterday) i visited rem and took
some cool things like gussdk (finally!) and mikmod203. tomorrow (today?)
i will recode this player... kool???

 5-08-97 22:15:47. huh, still nothing was done, coz i have to code an intro
for gravity. anyway, finally i managed to check dope.mod (28ch) with the
player and it is ok! cool...

 6-08-97 00:43:06. first changes in my gus code.... nothin' big yet.

  5-09-97 00:37:02, hahaha, gus unit still not modified... i had no time
and will to play with it. sea rulez, rite? anyway, now i'm going rather to
change the structure of the player than to modify/add drivers. there will
be time for this later, coz i'm going to use .dlls! it's very cooool thing,
but first i have to get some infos about this. so, time for a break.