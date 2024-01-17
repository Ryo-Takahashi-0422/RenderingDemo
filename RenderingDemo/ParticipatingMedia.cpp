#include <stdafx.h>
#include <ParticipatingMedia.h>

ParticipatingMedia ParticipatingMedia::calculateUnit()
{
	// 新たなParticipatingMediaElementsオブジェクトを生成して本体の内容をコピー、単位を計算する。imguiで変化可能にさせる想定。
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