# Direct Link: how the screens behave

The look is Pixelado's design system (pixelado-party `design/`): navy surfaces, the Pixel blue plate for what is
picked, pixel corners, Pixelify Sans. Direct Link draws it whatever the menu's theme is.

## The main screen, link off: Host and Join

Two big choices side by side on the bottom screen: **Host** (left) and **Join** (right). The mode (Wi-Fi) is not
shown: it is the only one that links today.

- **Nothing is picked when the app opens.** The legend's **Confirm** (A) is greyed until something is.
- **Buttons:** Left picks Host, Right picks Join; **A** confirms the one picked.
- **Touch:** a tap on a choice that is not picked **picks** it; a tap on the choice already picked **confirms** it.
  So from a fresh start, **two taps on the same choice start the link** (or Right/Left and A).
- What is picked shows as the accent plate (blue, dark text); the other stays on the card colour.
- No Wi-Fi radio: both choices greyed, the top screen says why.

## Starting or joining

- The picked choice shows its state (**Starting…** or **Joining…**); the other is greyed and does nothing.
- The legend has one key: **Cancel** (B, or a tap on it). It brings the link down (and home Wi-Fi back, if it was on);
  the screen shows **Cancelling…** until the link is down, then returns to the two choices with the same one picked.
- Exit and Settings wait until the start ends or is cancelled.

## The link up

The bottom screen becomes the link's card: who this console is (**You are the host** / **You are a guest**), the
state, and the addresses. The legend: **Exit** (B; the link stays up), **Settings** (START; greyed while linked),
**Disconnect** (X, its key in the design system's red "bad"), which asks first (A confirms, B cancels).

The **top screen** always says what is happening; the badge lives only there (the link's card below says it in
words, beside the characters):

| State | Badge | Line |
|---|---|---|
| Off | grey **Off** | Pick Host on one console and Join on the other. |
| Starting / joining | blue **Starting… / Joining…** | …you can cancel it. |
| Host, nobody yet | blue **Waiting…** | On the other console: Direct Link, then Join. |
| Host, someone on the link | green **Connected** | Open Pixelado Party or another app with LAN play… |
| Guest, host not heard yet | blue **Waiting…** | On the other console: Direct Link, then Host. |
| Guest, host heard | green **Connected** | Open Pixelado Party or another app with LAN play… |
| Guest, host quiet | blue **Searching…** | The host stopped answering. Looking for it again… |

The consoles' drawing sits centred between the two rules; how many are connected is said once, on the bottom card.

**The characters:** Nilo and Lumi beside the top screen's line while there is no link (also while it starts, joins or
is cancelled); on the link's card, this console's one (Nilo the host, Lumi a guest) and the other once it answers, with
the logo's spark between them; Pixo (from the chest up) in About. Their idle (a 1-px bob and a blink) redraws the
screen only when one of them changes pose, at most every 250 ms; the link's status is still read once a second.

**Who is connected** comes from wifi-link: on the host, every 5 s, the guests in its ARP table that answer a unicast
ping (`peers`, `peer_ips` in `status.sh`); a guest counts the host only while it answers (phase up, `peers=1`): up but
not hearing it yet (phase waiting) is **Waiting**, never Connected. Before this, the app
counted any ARP entry, even one left from a console that had gone.

## What it takes from the system

The drawing is all Direct Link's own (DRM straight to the panels, `pixel.c`), the same on any Linux handheld. From the
system it only reads:

- **The font** (Pixelify Sans): the system's copy (BananaOS: `/usr/share/banana/rnds/fonts`), else the app's own
  `fonts/`, which the build packs for a system that lacks it (SIL Open Font License).
- **The menu sounds:** the system's (BananaOS: its theme's, `/usr/share/banana/rnds/sounds`, and whether they are on);
  a system without them leaves the app silent.
- **Language and clock format:** the system's settings (BananaOS: `/data/banana/settings.conf`); English and 24 h when
  there are none.
