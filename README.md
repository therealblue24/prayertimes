# Prayer Times
Prayer time calculation software, coded in C.

Currently there is only a terminal CLI, library coming soon (TM)! 

## Features
- Calculating the 5 prayer times
	- And yes, Asr calculation is now fixed.
- Sunset & sunrise
- Midnight
- Imsak
- 12 & 24-hour support
- Customizable fajr, maghrib, isha angle
- Customizable asr shadow length
- Customizable maghrib, isha, imsak minutes
- Can select which times to show and not show
- Can show seconds
- Elevation support, increasing accuracy
- 7 calculation methods
- Prayer time adjustment, increasing accuracy

## Startup

Build the project using `make`.

Run `./bin/prayertimes` and it should guide you through the configuration process.

Run `./bin/prayertimes --reconf` to force the configuration process.

In the configuration process, it will ask you for your:

- Latitude & Longitude - for the calculation.
- Elevation (optional, put 0 if don't know or don't want) - to improve the accuracy of the calculation.
- Shia or Sunni - self explanatory.
- (for Sunni): Shafi'i and Hanafi - which school of thought do you follow (for Asr calculation).

After you are done configuring, the program should revert back to normal.

On normal circumstances, it should just print the following prayers/times and their times:

- Fajr
- Sunrise
- Dhuhr
- Asr
- Maghrib
- Isha

Please do note that even though efforts have been put to make the calculation as accurate as possible, there may be inaccuracies and you should NOT fully trust the times given to you by this program.

## Command Line Options

`prayertimes -h`:

```
Usage:
	-s, --silent			only print times
	-f, --show-future-only		show only future times
	-rc, --reconfigure		reconfigure location, method
	-ss, --sunset			print sunset time
	--imsak				print imsak time
	--midnight			print midnight
	-u, --utc			print times in UTC
	-h, --help			this page
	--version			print version of prayertimes
	-c, --color			colorize prayer times (requires truecolor support)
	--no-fajr			don't print fajr
	--no-sunrise			don't print sunrise
	--no-dhuhr			don't print dhuhr
	--no-asr			don't print asr
	--no-maghrib			don't print maghrib
	--no-isha			don't print isha
	--usage				this page
	-12h				print times in 12 hour format
	-24h				print times in 24 hour format
	--seconds			print seconds along with time
	-e, --elevation			set elevation
	-fa, --fajr-angle		set fajr angle
	-aa, --asr-shadow-length	set custom asr shadow length
	-ia, --isha-angle		set isha angle
	-mm, --maghrib-minutes		set maghrib minutes
	-ma, --maghrib-angle		set maghrib angle
	-im, --imsak-minutes		set imsak minutes
	-i,  --isha-minutes		set isha minutes
	--ramadan			pass this flag if it's Ramadan
	--adjust			adjust prayer times
	-mwl, --mwl-method		use MWL method
	-isna, --isna-method		use ISNA method
	-egypt, --egypt-method		use Egypt method
	-makkah, --makkah-method	use Makkah method
	-karachi, --karachi-method	use Karachi method
	-tehran, --tehran-method	use Tehran method
	-jafari, --jafari-method	use Jafari method
```
