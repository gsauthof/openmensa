.PHONY: srpm
srpm:
	dnf -y install git-core
	git archive --prefix openmensa-fra-bi/ -o openmensa-fra-bi.tar HEAD
	rpmbuild --define "_sourcedir $$PWD" --define "_specdir $$PWD" --define "_builddir $$PWD" --define "_rpmdir $(outdir)" --define "_srcrpmdir $(outdir)" -bs openmensa-fra-bi.spec
