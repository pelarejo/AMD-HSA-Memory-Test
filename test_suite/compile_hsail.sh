#!/bin/bash
for f in ./hsail/*.hsail
do
	HSAILasm $f
done
