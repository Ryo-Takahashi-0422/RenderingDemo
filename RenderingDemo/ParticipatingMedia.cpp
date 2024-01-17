#include <stdafx.h>
#include <ParticipatingMedia.h>

ParticipatingMedia ParticipatingMedia::calculateUnit()
{
	// �V����ParticipatingMediaElements�I�u�W�F�N�g�𐶐����Ė{�̂̓��e���R�s�[�A�P�ʂ��v�Z����Bimgui�ŕω��\�ɂ�����z��B
	ParticipatingMedia elements = *this;

	elements.rayleighScattering[0] *= 1e-6f;
	elements.rayleighScattering[1] *= 1e-6f;
	elements.rayleighScattering[2] *= 1e-6f;
	elements.mieScattering *= 1e-6f;
	elements.mieAbsorption *= 1e-6f;
	elements.ozoneAbsorption[0] *= 1e-6f;
	elements.ozoneAbsorption[1] *= 1e-6f;
	elements.ozoneAbsorption[2] *= 1e-6f;
	elements.altitudeOfRayleigh *= 1e3f;
	elements.altitudeOfMie *= 1e3f;
	elements.halfWidthOfOzone *= 1e3f;
	elements.altitudeOfOzone *= 1e3f;
	elements.groundRadius *= 1e3f;
	elements.atomosphereRadius *= 1e3f;

	return elements;
}