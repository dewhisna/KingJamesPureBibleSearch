#!/bin/sh

DIR=$(dirname "$0")

mkdir -p "${DIR}/festival"
cd "${DIR}/festival"

if [ ! -d "2.1" ]; then
	mkdir -p 2.1
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.1/speech_tools-2.1-release.tar.gz -o 2.1/speech_tools-2.1-release.tar.gz
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.1/festival-2.1-release.tar.gz -o 2.1/festival-2.1-release.tar.gz
fi

if [ ! -d "2.1/lexicon" ]; then
	mkdir -p 2.1/lexicon
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.1/festlex_CMU.tar.gz -o 2.1/lexicon/festlex_CMU.tar.gz
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.1/festlex_OALD.tar.gz -o 2.1/lexicon/festlex_OALD.tar.gz
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.1/festlex_POSLEX.tar.gz -o 2.1/lexicon/festlex_POSLEX.tar.gz
fi

if [ ! -d "2.1/voices" ]; then
	mkdir -p 2.1/voices
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.1/festvox_kallpc16k.tar.gz -o 2.1/voices/festvox_kallpc16k.tar.gz
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.1/festvox_rablpc16k.tar.gz -o 2.1/voices/festvox_rablpc16k.tar.gz
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.1/festvox_cmu_us_awb_cg.tar.gz -o 2.1/voices/festvox_cmu_us_awb_cg.tar.gz
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.1/festvox_cmu_us_rms_cg.tar.gz -o 2.1/voices/festvox_cmu_us_rms_cg.tar.gz
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.1/festvox_cmu_us_slt_arctic_hts.tar.gz -o 2.1/voices/festvox_cmu_us_slt_arctic_hts.tar.gz
fi

sha1sum -c 2.1-sha1sums || { echo 'sha1sums failed check after downloading' ; exit 1; }

echo "Extracting speech_tools:"
tar -zxf 2.1/speech_tools-2.1-release.tar.gz

echo "Patching speech_tools for gcc 4.8.2:"
patch -p1 < speech_tools-2.1.patch

echo "Patching speech_tools for gcc 7.4.0:"
patch -p1 < speech_tools-2.1-1.patch

echo "Building speech_tools:"
cd speech_tools
./configure
make
make install
cd ..

echo "Extracting festival:"
tar -zxf 2.1/festival-2.1-release.tar.gz
echo "Building festival:"
cd festival
./configure
make
cd ..

echo "Extracting lex/vox libraries:"
tar -zxf 2.1/lexicon/festlex_CMU.tar.gz
tar -zxf 2.1/lexicon/festlex_OALD.tar.gz
tar -zxf 2.1/lexicon/festlex_POSLEX.tar.gz
tar -zxf 2.1/voices/festvox_kallpc16k.tar.gz
tar -zxf 2.1/voices/festvox_rablpc16k.tar.gz
tar -zxf 2.1/voices/festvox_cmu_us_awb_cg.tar.gz
tar -zxf 2.1/voices/festvox_cmu_us_rms_cg.tar.gz
tar -zxf 2.1/voices/festvox_cmu_us_slt_arctic_hts.tar.gz

echo "Testing..."
cd festival
make test

echo '(SayText "Festival is ready")' | bin/festival --pipe

