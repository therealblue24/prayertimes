# Prayer Times
Prayer time calculation software

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

```
Usage:
	-s, --silent			only print times
	-f, --show-future-only		show only future times
	-rc, --reconfigure		reconfigure location, method
	-ss, --sunset			print sunset time
	-u, --utc			print times in UTC
	-h, --help			this page
	--usage				this page
	-12h				print times in 12 hour format
	-24h				print times in 24 hour format
	--seconds			print seconds along with time
```
