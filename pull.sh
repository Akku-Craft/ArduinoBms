#!/bin/bash

echo "--- Starte Pull von beiden GitHub-Repos ---"

# 1. Pull vom ersten Remote (meistens origin)
echo "Pulling from origin..."
git pull origin master  # Falls dein Branch 'master' heißt, bitte anpassen

echo "-------------------------------------------"

# 2. Pull vom zweiten Remote
echo "Pulling from second remote (backup)..."
git pull second-remote master

echo "--- Projekt ist nun auf dem Stand beider Repos ---"
read -p "Drücke Enter zum Beenden..."