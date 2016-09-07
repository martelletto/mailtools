# mailtools
a pair of tools to replace procmail in minimalistic e-mail setups

- deliver.c reads a message from stdin and places it in a maildir
  specified by argv[1].
- switch.lua decides to which maildir a message should go based on the
  message's SMTP headers and invokes deliver.c for delivery.

instructions:

- decide where 'deliver' and 'switch' will live.
- edit switch.lua accordingly and to your taste.
- run "make".
- install the generated tools.
