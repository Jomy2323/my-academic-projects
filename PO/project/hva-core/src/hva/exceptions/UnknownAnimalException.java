package hva.exceptions;

import java.io.Serial;

public class UnknownAnimalException extends SearchException {

	@Serial
	private static final long serialVersionUID = 202404101620L;

	/** @param key */
	public UnknownAnimalException(String key) {
    	super(key);
	}
}