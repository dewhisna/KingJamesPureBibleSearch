How to compile KingJamesPureBibleSearch GUI, WebChannel, and Command-Line Utilities
Using a Docker Container Image provisioned by Packer:

1. Install Docker
2. Install Packer
3. Build KJPBS Docker Image
4. Run KJPBS Tools

KJPBS Tools Built:
- KingJamesPureBibleSearch
- KJVDataDump
- KJVDataParse
- KJVDictWord
- KJVDiff
- KJVLookup
- KJVPhraseSearch
- KJVSearch
- KJVSumThing
- webchannel


================================================================================
Installing Docker on Ubuntu (See Docker website for other operating systems)
================================================================================
From: https://docs.docker.com/engine/install/ubuntu/


Remove Old Dockers:
$ sudo apt-get remove docker docker-engine docker.io containerd runc

Install Dependencies:
$ sudo apt-get update
$ sudo apt-get install \
    ca-certificates \
    curl \
    gnupg \
    lsb-release \
    apt-transport-https \
    software-properties-common

Add Signature:
$ curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg
$ sudo apt-key fingerprint 0EBFCD88

Verify:
pub   rsa4096 2017-02-22 [SCEA]
      9DC8 5822 9FC7 DD38 854A  E2D8 8D81 803C 0EBF CD88
uid           [ unknown] Docker Release (CE deb) <docker@docker.com>
sub   rsa4096 2017-02-22 [S]

Add Repository:
$ echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://download.docker.com/linux/ubuntu \
  $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

Install Docker:
$ sudo apt-get update
$ sudo apt-get install docker-ce docker-ce-cli containerd.io

Add yourself to the Docker Group:
$ sudo usermod -aG docker your-user-id
(Logout and back in to enable group)

Verify Installation:
$ docker run --rm hello-world

================================================================================


================================================================================
Install Packer
================================================================================

Download latest Packer zip file from:
https://www.packer.io/downloads.html

Unzip it and copy it to a folder on your system search path:
$ sudo cp packer /usr/local/bin

================================================================================


================================================================================
Build KJPBS Docker Image
================================================================================

You can either do a 'git clone' of the entire KJPBS Repository, or you can
just download the contents of the 'docker' subfolder.  If you do the latter,
be sure to navigate to the 'raw' file content and don't accidentally save
the GitHub generated webpage rendered view of the files or it won't work.

$ cd KingJamesPureBibleSearch/docker
(from Git clone location)

Run Packer to generate the Docker Image:
$ packer build packer_docker_KJPBS.json
(and take a coffee break... this step takes a while)

================================================================================


================================================================================
Run KJPBS Tools
================================================================================

Once you have created the image, start a new Docker container from this image
using:
$ docker run -d --restart=always --name kjpbs localhost:5000/dewtronics/kjpbs:latest

This will start a new container named 'kjpbs' running in the background.

To login to the container, use this command:
$ docker exec -it kjpbs startup

This will execute the special startup script inside the container named 'kjpbs' in
an interactive terminal.  When starting the container in this fashion it should
not prompt for a password and most operations you do with the KJPBS tools will
not require any password.

However, the initial password for the 'kjpbs' user in the Docker container is
an empty string.  So, if you want to experiment with other container features,
such as log into the container with RDP (Remote Desktop Protocol) to run the
GUI version of KJPBS inside the container, or if you need to use any of the
'sudo' commands, just press enter if prompted for a password to enter an empty
string.

If you wish to actually set a non-empty password for some reason, use the
'passwd' command within the container.  But note that it will remain set only as
long as the container exists.  If you remove the container, any settings inside
the container are lost.

Now that you've logged into the container, you can run the tools.  For example:
$ KJVLookup 1 "John 3:16"

$ KJVSearch -h -t 1 "Word of God"

$ KJVPhraseSearch -sc 1 13

$ KJVSumThing -sc -om 1 1 37

Run these commands without any arguments for usage information and help
instructions.

Enter 'exit' when finished to exit your login shell to the container:
$ exit


This will only exit your login, but the Docker Container will remain running
in the background.  You can list the containers and check their status using:
$ docker container ls --all


And you can log back into this container anytime with the 'exec' command:
$ docker exec -it kjpbs startup 


You can stop the KJPBS Docker Container with:
$ docker stop kjpbs


And you can remove the KJPBS Docker Container (after stopping it) with:
$ docker rm kjpbs


Note that once you've created the KJPBS Docker Image, as long as you don't
remove it (even if you stop and remove all containers that were running the
image), there is no need to regenerate the entire image each time you want
to use KJPBS.  You can just start with this step of "Running KJPBS Tools"
with a Docker Container.

You can list the installed Docker Images with:
$ docker images


Later on, if you decide to upgrade your KJPBS Docker Image to get a new
version of KJPBS and its tools, don't forget to remove any existing KJPBS
containers and images or you will end up wasting space on your drive with
old images that you no longer use.

Stop and remove just the kjpbs container with:
$ docker stop kjpbs
$ docker rm kjpbs

And remove the old image with:
$ docker image rm localhost:5000/dewtronics/kjpbs:latest


If you are only using Docker for running KJPBS, you can remove all old
containers using:
$ docker container prune

And then remove all old images with:
$ docker image prune

Note that until you remove the images, you can still use them to launch
new containers.  So make sure you are done with an image before deleting it.

================================================================================


================================================================================
Run KJPBS WebChannel and the full KJPBS GUI inside the container
================================================================================

Launching KJPBS Web Channel:
----------------------------
After you've created your KJPBS Docker Image, if you want to run the KJPBS
WebChannel locally on your PC from within the container, you will need to first
stop and remove any containers you have launched with the instructions above with:
$ docker stop kjpbs
$ docker rm kjpbs

The reason, to stop the container is to launch it with the web ports mapped to
your host PC's localhost to easily access it from your browser.  So start the
container with both ports 80 and 9340 mapped to your host computer:
$ docker run -p 80:80 -p 9340:9340 -d --restart=always --name kjpbs localhost:5000/dewtronics/kjpbs:latest

This assumes you aren't currently using port 80 on your host PC.  If you are,
replace the first '80' with something else like '8080':
$ docker run -p 8080:80 -p 9340:9340 -d --restart=always --name kjpbs localhost:5000/dewtronics/kjpbs:latest

(Note port 9340 could also be remapped to something different, but then you'd
have to pass that to the WebChannel web page when launching it.  Since it's not
likely port 9340 would already be in use, those instructions aren't included here)


Then login to the container:
$ docker exec -it kjpbs startup

And start the KJPBS WebChannel server:
$ webchannel

Bring up your favorite Web Browser on your PC and navigate to:
http://localhost/

Or:
http://localhost:8080/
If you remapped to port 8080 above (or whatever number you used instead).

And you should now be running the full WebChannel application through your browser.

To stop the webchannel, you can press ctrl-C in the terminal where you ran
'webchannel' from.  Or, you could use the "more correct" method by sending a
SIGHUP (hang-up signal) to the webchannel process either from another login
session with the container.  Or by an 'exec' command:
$ docker exec kjpbs kill -SIGHUP `docker exec kjpbs pidof KingJamesPureBibleSearch_webchannel`

If for some reason the webchannel page doesn't load in your browser, you can try
to restart the 'nginx' webserver in the container by logging into the container
in another terminal window and using:
$ sudo nginx -s stop
$ sudo nginx

And then trying again to access the page.  You should not have to restart the
KJPBS WebChannel app with nginx, as it runs as an independent server and was
started by you manually with the 'webchannel' command, whereas the 'nginx' server
is started automatically as a SystemD daemon (see notes below for KJPBS GUI).

It probably won't even be necessary to restart 'nginx' because, for some reason,
it seems to be a well behaved daemon with respect to Docker Containers and the
docker-systemctl-replacement script I'm using to autolaunch multi-user system
daemons.


Running KJPBS GUI:
------------------
First install a suitable RDP client on your host PC.  For Linux, KRDC is a decent
client.  Windows comes with a 'Remote Desktop Connection' application.  And Mac
has a number of freely available clients as well.

After you've created your KJPBS Docker Image, if you want to run the KJPBS GUI
locally on your PC from within the Container using RDP, you will need to first
stop and remove any containers you have launched with the instructions above with:
$ docker stop kjpbs
$ docker rm kjpbs

The reason, to stop the container is to launch it with the 3389 RDP port mapped to
your host PC's localhost to easily access it from your host PC.  So start the container with:
$ docker run -p 3389:3389 -d --restart=always --name kjpbs localhost:5000/dewtronics/kjpbs:latest

This assumes that you aren't currently using port 3389 on your host PC.  If you are,
replace the first 3389 with a different port, and specify that port to your RDP
Client.  For example:
$ docker run -p 8000:3389 -d --restart=always --name kjpbs localhost:5000/dewtronics/kjpbs:latest

Then login to the container:
$ docker exec -it kjpbs startup

The next step is a little tricky and is only because it's difficult to handle the
startup/shutdown logic for running multi-user SystemD daemons inside a Docker
Container.

Check the system processes:
$ ps auxww

And make sure you see both '/usr/sbin/xrdp' and '/usr/sbin/xrdp-sesman' running.
If only one of the two is running or neither is running, first stop xrdp with:
$ sudo systemctl stop xrdp
$ sudo systemctl stop xrdp-sesman

Then start the xrdp server with:
$ sudo /etc/init.d/xrdp start

Now when you check the system processes, you should see both listed.

Then, bring up your RDP Client on your host PC and connect to:
kjpbs@localhost:3389
Using the RDP protocol (replace the 3389 with whatever number you binded above
when running the container, if you used a number other than 3389).

Depending on your RDP client, the above may be entered in a single connection
string, or you may have to enter them as separate fields.  The username is 'kjpbs'.
The URL is 'localhost'.  And the port is '3389'.  And if prompted for a password,
the initial password for 'kjpbs' in the container is an empty string, so leave
it blank (unless you manually changed it with the 'passwd' command in the container).

It's recommended that you use at least a 1280x1024 display size setting on your
RDP client (or larger).  Once logged in, select to create the 'default' panel config
and you'll be looking at the Xubuntu Xfce desktop.  Most of the applications listed
in the menu will NOT work.  So don't be surprised if you get an error when you
select them.  This Docker Container Image is quite sparse and only contains the
essential pieces for KJPBS.  If you want the other apps to be available, you'll
need to manually use 'sudo apt-get install' from the container's command-line
to install them.

To Launch KJPBS, select "Run Program..." from the Appplications Menu drop list
in the upper-left-hand corner, and enter: "KingJamesPureBibleSearch".
This will launch the complete KJPBS GUI for Linux as if you had installed it
directly on your own PC, only it's running completely inside the Docker
Container, much like a Virtual Machine.

You have to manually enter 'KingJamesPureBibleSearch' in the 'Run Program...'
window because there is currently no icon installed in the menus for KJPBS,
since this Docker image was a raw source build of KJPBS and not from the
installer.

When done, either 'logout' from the Xfce desktop or just drop the RDP
connection.  Note that if you just drop the RDP connection, but stay logged
in with KJPBS and such running, it will still be running right where you left
off when you reconnect -- unless you shutdown or otherwise disturb the Docker
Container.

================================================================================


================================================================================
Run Everything
================================================================================

If you want to be able to run all of the tools at the same time, note that you
can combine the instructions from above and start your container with all
needed ports mapped to your localhost at the same time, by using:

$ docker run -p 80:80 -p 9340:9340 -p 3389:3389 -d --restart=always --name kjpbs localhost:5000/dewtronics/kjpbs:latest

That way there will be no needed to stop and remove the existing container and
map different ports.  You'll have one container that can run everything.

================================================================================

