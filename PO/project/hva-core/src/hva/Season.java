package hva;

public enum Season {
    SPRING,
    SUMMER,
    FALL,
    WINTER;

    public Season next() {
        return values()[(ordinal() + 1) % values().length];
    }
}

