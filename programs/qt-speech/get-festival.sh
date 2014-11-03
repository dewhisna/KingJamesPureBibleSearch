#!/bin/sh
mkdir -p festival
cd festival
curl http://www.cstr.ed.ac.uk/downloads/festival/2.1/speech_tools-2.1-release.tar.gz -o speech_tools.tar.gz
tar -zxf speech_tools.tar.gz
cd speech_tools
./configure
make
make install
cd ..
curl http://www.cstr.ed.ac.uk/downloads/festival/2.1/festival-2.1-release.tar.gz -o festival.tar.gz
tar -zxf festival.tar.gz
cd festival
./configure
make

cd ..
curl http://www.cstr.ed.ac.uk/downloads/festival/2.1/festlex_CMU.tar.gz -o festlex_CMU.tar.gz
curl http://www.cstr.ed.ac.uk/downloads/festival/2.1/festlex_OALD.tar.gz -o festlex_OALD.tar.gz
curl http://www.cstr.ed.ac.uk/downloads/festival/2.1/festlex_POSLEX.tar.gz -o festlex_POSLEX.tar.gz
curl http://www.cstr.ed.ac.uk/downloads/festival/2.1/festvox_kallpc16k.tar.gz -o festvox_kallpc16k.tar.gz
curl http://www.cstr.ed.ac.uk/downloads/festival/2.1/festvox_rablpc16k.tar.gz -o festvox_rablpc16k.tar.gz

tar -zxf festlex_CMU.tar.gz
tar -zxf festlex_OALD.tar.gz
tar -zxf festlex_POSLEX.tar.gz
tar -zxf festvox_kallpc16k.tar.gz
tar -zxf festvox_rablpc16k.tar.gz

cd festival
make test

echo '(SayText "Festival is ready")' | bin/festival --pipe

