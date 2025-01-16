package hva.exceptions;

import java.io.Serial;

public class UnknownEmployeeException extends SearchException {

	@Serial
	private static final long serialVersionUID = 202404101621L;

	/** @param key */
	public UnknownEmployeeException(String key) {
    	super(key);
	}
}