package hva.app.search;

import hva.exceptions.SearchException;

import java.util.List;

public interface SearchStrategy {
    List<String> execute() throws SearchException;
}

