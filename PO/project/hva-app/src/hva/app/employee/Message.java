package hva.app.employee;

public interface Message {
    /**
   * @return tag for individual vet
   */
  static String typeVet() {
    return "VET";
  }

  /**
   * @return tag for infividual handler
   */
  static String typeHandler() {
    return "TRT";
  }
}