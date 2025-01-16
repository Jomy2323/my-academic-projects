package hva;

import java.io.Serial;
import java.io.Serializable;
import java.util.Map;

import hva.satisfaction.SatisfactionCalculator;

public abstract class Employee implements Serializable{

    /** employee's id */
    private String _employeeId;

    /** employee's name */
    private String _employeeName;

    /** employee's satisfaction calculator */
    private SatisfactionCalculator _employeeSatisfactionCalculator;

    /**
     * Constructor
     * @param id the employee's id
     * @param name the employee's name
     */
    public Employee(String id, String name){
        _employeeId = id;
        _employeeName = name;
    }

    //getter methods
    public String getEmployeeId(){
        return _employeeId;
    }

    public int getSatisfaction() {
        return _employeeSatisfactionCalculator.
         calculateSatisfaction();
    }

    //setter methods
    public void setSatisfactionCalculator(SatisfactionCalculator
     calculator) {
        _employeeSatisfactionCalculator = calculator;
    }

    @Override
    public String toString() {
        return _employeeId + "|" + _employeeName;
    }
}