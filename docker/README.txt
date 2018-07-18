How to compile KingJamesPureBibleSearch Command-Line Utilities Using a
Docker Container provisioned by Packer:

1. Installer Docker
2. Install Packer
3. Build Docker Image
4. Run KJPBS Tools

Tools Built:
- KJVLookup
- KJVSearch
- KJVPhraseSearch
- webchannel


================================================================================
Installing Docker on Ubuntu
================================================================================
From: https://docs.docker.com/install/linux/docker-ce/ubuntu/


Remove Old Dockers:
$ sudo apt-get remove docker docker-engine docker.io

Install Dependencies:
$ sudo apt-get update
$ sudo apt-get install \
    apt-transport-https \
    ca-certificates \
    curl \
    software-properties-common

Add Signature:
$ curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
$ sudo apt-key fingerprint 0EBFCD88

Verify:
pub   4096R/0EBFCD88 2017-02-22
      Key fingerprint = 9DC8 5822 9FC7 DD38 854A  E2D8 8D81 803C 0EBF CD88
uid                  Docker Release (CE deb) <docker@docker.com>
sub   4096R/F273FCD8 2017-02-22

Add Repository:
$ sudo add-apt-repository \
   "deb [arch=amd64] https://download.docker.com/linux/ubuntu \
   $(lsb_release -cs) \
   stable"

Install Docker:
$ sudo apt-get update
$ sudo apt-get install docker-ce

Add yourself to the Docker Group:
$ sudo usermod -aG docker your-user
(Logout and back in to enable group)

Verify Installation:
$ docker run --rm hello-world

==================================================================================


================================================================================
Install Packer
================================================================================

Download latest Packer zip file from:
https://www.packer.io/downloads.html

Unzip and:
$ sudo cp packer /usr/local/bin

==================================================================================


================================================================================
Build KJPBS Docker Image
================================================================================

$ cd KingJamesPureBibleSearch/docker
(from Git clone location)

$ packer build packer_docker_KJPBS.json
(and take a coffee break... this step takes a while)

==================================================================================


================================================================================
Run KJPBS Tools
================================================================================

Start the Docker container:
$ docker run --rm -it localhost:5000/dewtronics/kjpbs-cmd:latest

Now, run the tools.  For example:
# KJVLookup 1 "John 3:16"

# KJVSearch -h -t 1 "Word of God"

# KJVPhraseSearch -sc 1 13

Run them without any arguments for usage information.

Enter 'exit' when finished to shutdown the container:
# exit

==================================================================================


