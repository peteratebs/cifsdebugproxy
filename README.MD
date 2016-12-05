* Copyright, Peter Van Oudenaren
*  EBS Inc. 1987-2015
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.

* contact sales@ebsembeddedsoftware.com

build
  cd ./csdebugproxy/http/v0.2.6/project/linux_generic/linuxserver
  make clean
  make

Run:
  ./run.sh


Web Access:
  locally:  http://localhost:8080/Html4Demo.html
  remotely: example: http://192.168.1.5:8080/Html4Demo.html

Usage:
  click on one of the SMB command buttons and a message is routed to the smb server to dump diagnostices.
  Needs work, just a shell now.

