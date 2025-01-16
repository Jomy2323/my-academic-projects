package hva.exceptions;

import java.io.Serial;

public class VeterinarianNotAuthorizedException extends Exception {

	@Serial
	private static final long serialVersionUID = 202404101628L;

	/** The veterinarian's key. */
	private final String _vetKey;
	private final String _speciesKey;

	/** 
	 * @param vetKey 
	 * @param speciesKey
	*/
	public VeterinarianNotAuthorizedException(String vetKey,
	 String speciesKey) {
		_vetKey = vetKey;
		_speciesKey = speciesKey;
	}

  	/** @return the vetKey */
  	public String getVetKey() {
		return _vetKey;
  	}

  	/** @return the speciesKey */
  	public String getSpeciesKey() {
		return _speciesKey;
  	}
}