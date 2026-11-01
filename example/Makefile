MPREMOTE ?= mpremote

.PHONY: run deploy repl ls reset

run:
	PYTHONUNBUFFERED=1 $(MPREMOTE) run main.py

deploy:
	$(MPREMOTE) fs cp main.py :main.py

repl:
	$(MPREMOTE) repl

ls:
	$(MPREMOTE) fs ls

reset:
	$(MPREMOTE) reset
