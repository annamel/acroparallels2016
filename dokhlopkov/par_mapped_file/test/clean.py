import os

if os.path.isfile("big"):
    os.remove("big")
if os.path.isfile("small"):
    os.remove("small")
if os.path.isfile("temp"):
    os.remove("temp")
