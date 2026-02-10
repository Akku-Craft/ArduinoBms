#!/bin/bash

# 1. Zuerst deine Änderungen sammeln (Vorbereiten)
git add .

# 2. Den Commit ausführen (Lokal speichern)
echo "Enter commit message:"
read message
git commit -m "$message" || { echo "Nichts zu committen oder Fehler!"; }

# 3. JETZT den Stand von GitHub holen und deine Arbeit oben drauf setzen
echo "Checking for updates from GitHub..."
git pull origin master --rebase || { echo "Fehler beim Pull! Konflikte müssen manuell gelöst werden."; exit 1; }

# 4. Zum ersten Repo pushen (origin)
echo "Pushing to origin..."
git push origin master

# 5. Zum zweiten Repo pushen (second-remote)
echo "Pushing to second-remote..."
git push second-remote master

echo "Done! Everything is synced."d."