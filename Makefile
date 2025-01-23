
default:
	pio run -e c3

flash:
	pio run -e c3 -t upload

monitor:
	pio device monitor


