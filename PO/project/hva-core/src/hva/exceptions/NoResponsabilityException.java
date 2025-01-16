package hva.exceptions;

import java.io.Serial;

public class NoResponsabilityException extends Exception {

	@Serial
	private static final long serialVersionUID = 202404101619L;

	/** The Responsability's key. */
	private final String _responsabilityKey;
	private final String _employeeKey;

	/** @param key */
	public NoResponsabilityException(String responsibilityKey,
	 String employeeKey) {
		_responsabilityKey = responsibilityKey;
		_employeeKey = employeeKey;
	}

  	/** @return the key */
  	public String getResponsabilityKey() {
    	return _responsabilityKey;
  	}

	/** @return the key */
	public String getEmployeeKey() {
		return _employeeKey;
  	}
}