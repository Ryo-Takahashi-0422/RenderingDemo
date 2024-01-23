#pragma once

struct ParticipatingMedia
{
	float rayleighScattering[3] = { 5.802f,13.558f,33.1f };
	float mieScattering = 3.996f ;
	float mieAbsorption = 4.4f ;
	float ozoneAbsorption[3] = { 0.650f,1.881f,0.085f };
	float asymmetryParameter = 0.8f;
	float altitudeOfRayleigh = 8.0f;
	float altitudeOfMie = 1.2f;
	float halfWidthOfOzone = 15.0f;
	float altitudeOfOzone = 25.0f;

	float groundRadius = 6360.0f;
	float atomosphereRadius = 6460.0f;
	//float pad = 0;

	ParticipatingMedia calculateUnit();
};