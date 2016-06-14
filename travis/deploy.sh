#!/bin/bash
set -e # Exit with nonzero exit code if anything fails


echo "script started"

# generate the manifest
python $HOME/scripts/buildmanifest.py /tmp/package /tmp/package/manifest.json

#  no host checking
#ssh -v -p 4022 -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -i /tmp/travis.key $HOME_USER@$HOME_IP "mkdir -p ~/projects/$TRAVIS_REPO_SLUG/$TRAVIS_BRANCH/latest/"
#scp -v -P 4022 -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -i /tmp/travis.key -rp /tmp/package/. "$HOME_USER@$HOME_IP:~/projects/$TRAVIS_REPO_SLUG/$TRAVIS_BRANCH/latest/"
#echo "repo slug = $TRAVIS_REPO_SLUG"

echo "DEPLOY HOME"

ssh -v -p 4022  -i /tmp/travis.key $HOME_USER@www.amelvin.co.uk "mkdir -p ~/projects/$TRAVIS_REPO_SLUG/$TRAVIS_BRANCH/$1/"
scp -v -P 4022  -i /tmp/travis.key -rp /tmp/package/. "$HOME_USER@www.amelvin.co.uk:~/projects/$TRAVIS_REPO_SLUG/$TRAVIS_BRANCH/$1/"
