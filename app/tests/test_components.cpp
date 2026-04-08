#include <gtest/gtest.h>
#include <vector>
#include <stdexcept>
#include "electronics_manager.hpp"
#include "electronic_math.hpp"

using namespace ecim;

// ── Helper: build a ComponentBaseConfig quickly ──────────────────────────────
static ElectronicComponent::BaseConfig cfg(
    const string& name,
    const string& mfg, const string& pn, const string& desc,
    const ElectronicRating& rating, size_t qty = 1)
{
    return ElectronicComponent::BaseConfig{rating, name, mfg, pn, desc, qty};
}

static const ElectronicRating kDefaultRating{5.0, 0.01, 0.05};

// ── Component construction ────────────────────────────────────────────────────

TEST(ComponentConstruction, Resistor) {
    Resistor r1(cfg("R1", "Vishay", "V-RES-1K", "1k resistor", kDefaultRating), 1000.0, 0.05);
    EXPECT_EQ(r1.name(),          "R1");
    EXPECT_EQ(r1.manufacturer(),  "Vishay");
    EXPECT_EQ(r1.partNumber(),    "V-RES-1K");
    EXPECT_EQ(r1.quantity(),      static_cast<size_t>(1));
    EXPECT_NEAR(r1.resistance(),    1000.0, 1e-9);
    EXPECT_NEAR(r1.toleranceBand(), 0.05,   1e-9);
}

TEST(ComponentConstruction, Capacitor) {
    Capacitor c1(cfg("C1", "Murata", "M-CAP-1uF", "1uF cap", kDefaultRating), Capacitor::Type::Ceramic, 1e-6);
    EXPECT_EQ(c1.name(),          "C1");
    EXPECT_EQ(c1.capacitorType(), Capacitor::Type::Ceramic);
    EXPECT_NEAR(c1.capacitance(), 1e-6, 1e-18);
}

TEST(ComponentConstruction, Inductor) {
    Inductor l1(cfg("L1", "CoilCo", "L-10uH", "10uH inductor", kDefaultRating), 10e-6);
    EXPECT_EQ(l1.name(), "L1");
    EXPECT_NEAR(l1.inductance(), 10e-6, 1e-18);
}

TEST(ComponentConstruction, Diode) {
    Diode d1(cfg("D1", "DiodeInc", "D-1N4148", "Signal diode", kDefaultRating), 0.7, Diode::Type::Regular);
    EXPECT_EQ(d1.name(),      "D1");
    EXPECT_EQ(d1.diodeType(), Diode::Type::Regular);
    EXPECT_NEAR(d1.forwardVoltage(), 0.7, 1e-9);
}

TEST(ComponentConstruction, BJTransistor) {
    BJTransistor t1(cfg("T1", "TransCo", "T-NPN-1", "NPN transistor", kDefaultRating), 100.0);
    EXPECT_EQ(t1.name(), "T1");
    EXPECT_NEAR(t1.gain(), 100.0, 1e-9);
}

TEST(ComponentConstruction, FETransistor) {
    FETransistor m1(cfg("M1", "MosCo", "M-NMOS-1", "N-MOSFET", kDefaultRating), 2.5);
    EXPECT_EQ(m1.name(), "M1");
    EXPECT_NEAR(m1.thresholdVoltage(), 2.5, 1e-9);
}

TEST(ComponentConstruction, IntegratedCircuit) {
    IntegratedCircuit ic1(cfg("IC1", "ICMaker", "IC-555", "Timer IC", kDefaultRating), 8, 5.0, 3.0, 1.5);
    EXPECT_EQ(ic1.name(),     "IC1");
    EXPECT_EQ(ic1.pinCount(), static_cast<size_t>(8));
    EXPECT_NEAR(ic1.width(),  5.0, 1e-9);
    EXPECT_NEAR(ic1.height(), 3.0, 1e-9);
    EXPECT_NEAR(ic1.length(), 1.5, 1e-9);
}

// ── Quantity management ───────────────────────────────────────────────────────

TEST(QuantityManagement, AddAndRemove) {
    Resistor qr(cfg("QR", "Test", "T-1", "", kDefaultRating, 10), 100.0, 0.01);
    EXPECT_EQ(qr.quantity(), static_cast<size_t>(10));
    qr.addQuantity(5);
    EXPECT_EQ(qr.quantity(), static_cast<size_t>(15));
    qr.removeQuantity(3);
    EXPECT_EQ(qr.quantity(), static_cast<size_t>(12));
}

TEST(QuantityManagement, OverRemoveThrows) {
    Resistor qr(cfg("QR2", "Test", "T-1", "", kDefaultRating, 10), 100.0, 0.01);
    EXPECT_THROW(qr.removeQuantity(999), std::exception);
}

// ── Validation errors ─────────────────────────────────────────────────────────

TEST(Validation, NegativeVoltageRatingThrows) {
    ElectronicRating bad{-1.0, 0.01, 0.05};
    EXPECT_THROW((Resistor{cfg("X", "M", "P", "", bad), 100.0, 0.01}), std::exception);
}

TEST(Validation, EmptyNameThrows) {
    EXPECT_THROW((Resistor{cfg("", "M", "P", "", kDefaultRating), 100.0, 0.01}), std::exception);
}

TEST(Validation, NegativeResistanceThrows) {
    EXPECT_THROW((Resistor{cfg("X", "M", "P", "", kDefaultRating), -1.0, 0.01}), std::exception);
}

TEST(Validation, NegativeCapacitanceThrows) {
    EXPECT_THROW((Capacitor{cfg("X", "M", "P", "", kDefaultRating), Capacitor::Type::Ceramic, -1e-6}), std::exception);
}

TEST(Validation, ZeroPinCountThrows) {
    EXPECT_THROW((IntegratedCircuit{cfg("X", "ICMaker", "IC-555", "Timer IC", kDefaultRating), 0, 1.0, 1.0, 1.0}), std::exception);
}

// ── ElectronicsManager ────────────────────────────────────────────────────────
// The manager is a singleton so all manager tests run in one TEST to keep state
// predictable and avoid cross-test interference.

TEST(ElectronicsManager, AddRetrieveAndStress) {
    ElectronicsManager& mgr = ElectronicsManager::instance();

    // Record baseline so these tests are independent of prior singleton state
    std::vector<ElectronicComponent*> baseline;
    mgr.getAllComponents(baseline);
    const size_t base = baseline.size();

    std::vector<ElectronicComponent*> base_res;
    mgr.getAllComponentsByType(ElectronicComponent::Type::Resistor, base_res);
    const size_t base_r = base_res.size();

    // Add one of every type
    mgr.addComponent(std::make_unique<Resistor>     (cfg("R2",  "Vishay",   "V-RES-4K7",  "4.7k resistor",   kDefaultRating, 5), 4700.0,  0.01));
    mgr.addComponent(std::make_unique<Resistor>     (cfg("R3",  "Yageo",    "RC-10K",     "10k resistor",    kDefaultRating, 3), 10000.0, 0.05));
    mgr.addComponent(std::make_unique<Resistor>     (cfg("R4",  "KOA",      "RK-47K",     "47k resistor",    kDefaultRating, 2), 47000.0, 0.05));
    mgr.addComponent(std::make_unique<Capacitor>    (cfg("C2",  "Murata",   "M-CAP-10n",  "10nF cap",        kDefaultRating, 10), Capacitor::Type::Ceramic, 10e-9));
    mgr.addComponent(std::make_unique<Capacitor>    (cfg("C3",  "KEMET",    "K-CAP-100u", "100uF electro",   kDefaultRating, 4), Capacitor::Type::AluminumElectrolytic, 100e-6));
    mgr.addComponent(std::make_unique<Inductor>     (cfg("L2",  "CoilCo",   "L-100uH",    "100uH inductor",  kDefaultRating, 2), 100e-6));
    mgr.addComponent(std::make_unique<Inductor>     (cfg("L3",  "CoilCo",   "L-1mH",      "1mH inductor",    kDefaultRating, 1), 1e-3));
    mgr.addComponent(std::make_unique<Diode>        (cfg("D2",  "DiodeInc", "D-1N5819",   "Schottky diode",  kDefaultRating, 6), 0.3, Diode::Type::Schottky));
    mgr.addComponent(std::make_unique<BJTransistor> (cfg("T2",  "TransCo",  "T-PNP-1",    "PNP transistor",  kDefaultRating, 3), 80.0));
    mgr.addComponent(std::make_unique<FETransistor>     (cfg("M2",  "MosCo",    "M-IRF540",   "Power N-MOSFET",  kDefaultRating, 2), 4.0));
    mgr.addComponent(std::make_unique<IntegratedCircuit>(cfg("IC2", "ICMaker", "IC-NE556", "Dual timer IC", kDefaultRating, 1), 14, 5.0, 4.0, 2.0));

    std::vector<ElectronicComponent*> all;
    mgr.getAllComponents(all);
    EXPECT_EQ(all.size(), base + 11);

    std::vector<ElectronicComponent*> mgr_resistors;
    mgr.getAllComponentsByType(ElectronicComponent::Type::Resistor, mgr_resistors);
    EXPECT_EQ(mgr_resistors.size(), base_r + 3);

    std::vector<ElectronicComponent*> mgr_caps;
    mgr.getAllComponentsByType(ElectronicComponent::Type::Capacitor, mgr_caps);
    EXPECT_EQ(mgr_caps.size(), static_cast<size_t>(2));

    std::vector<ElectronicComponent*> mgr_inductors;
    mgr.getAllComponentsByType(ElectronicComponent::Type::Inductor, mgr_inductors);
    EXPECT_EQ(mgr_inductors.size(), static_cast<size_t>(2));

    std::vector<ElectronicComponent*> mgr_mosfets;
    mgr.getAllComponentsByType(ElectronicComponent::Type::FETransistor, mgr_mosfets);
    EXPECT_EQ(mgr_mosfets.size(), static_cast<size_t>(1));

    // ── Series resistance on the 3 resistors just added ──────────────────────
    {
        // mgr_resistors holds base_r + 3 entries; the last 3 are R2/R3/R4
        vector<const Resistor*> rp;
        for (size_t i = mgr_resistors.size() - 3; i < mgr_resistors.size(); ++i)
            rp.push_back(static_cast<const Resistor*>(mgr_resistors[i]));
        // R2=4700 + R3=10000 + R4=47000 = 61700
        EXPECT_NEAR(ecmath::computeSeriesResistance(rp), 61700.0, 1e-9);
    }

    // ── Stress test (500 more resistors) ─────────────────────────────────────
    for (int i = 0; i < 500; ++i) {
        string name = "SR" + std::to_string(i);
        mgr.addComponent(std::make_unique<Resistor>(
            cfg(name, "StressMfg", "S-RES", "stress", kDefaultRating), 100.0 * (i + 1), 0.01));
    }

    std::vector<ElectronicComponent*> stress_res;
    mgr.getAllComponentsByType(ElectronicComponent::Type::Resistor, stress_res);
    EXPECT_EQ(stress_res.size(), base_r + 3 + 500);

    std::vector<ElectronicComponent*> all_after;
    mgr.getAllComponents(all_after);
    EXPECT_EQ(all_after.size(), base + 11 + 500);

    // removeComponent with invalid id — expect false
    EXPECT_FALSE(mgr.removeComponent(99999));
}

// ── ecmath — voltage divider ──────────────────────────────────────────────────

TEST(EcMath, VoltageDivider) {
    // Vout = Vin * R2 / (R1 + R2)
    EXPECT_NEAR(ecmath::computeVoltageDivider(12.0, 1000.0, 2000.0), 8.0,  1e-9);
    EXPECT_NEAR(ecmath::computeVoltageDivider(5.0,  500.0,  500.0),  2.5,  1e-9);
}

TEST(EcMath, VoltageDividerZeroResistanceThrows) {
    EXPECT_THROW(ecmath::computeVoltageDivider(5.0, 0.0, 0.0), std::exception);
}

// ── ecmath — resistance ───────────────────────────────────────────────────────

TEST(EcMath, SeriesResistance) {
    Resistor ra(cfg("RA", "T", "T", "", kDefaultRating), 300.0, 0.01);
    Resistor rb(cfg("RB", "T", "T", "", kDefaultRating), 700.0, 0.01);
    vector<const Resistor*> rp = {&ra, &rb};
    EXPECT_NEAR(ecmath::computeSeriesResistance(rp), 1000.0, 1e-9);
}

TEST(EcMath, ParallelResistance) {
    // 1/(1/300 + 1/700) = 210
    Resistor ra(cfg("RA2", "T", "T", "", kDefaultRating), 300.0, 0.01);
    Resistor rb(cfg("RB2", "T", "T", "", kDefaultRating), 700.0, 0.01);
    {
        vector<const Resistor*> rp = {&ra, &rb};
        EXPECT_NEAR(ecmath::computeParallelResistance(rp), 210.0, 1e-9);
    }
    // Two equal 1k → 500
    Resistor re1(cfg("RE1", "T", "T", "", kDefaultRating), 1000.0, 0.01);
    Resistor re2(cfg("RE2", "T", "T", "", kDefaultRating), 1000.0, 0.01);
    {
        vector<const Resistor*> rp = {&re1, &re2};
        EXPECT_NEAR(ecmath::computeParallelResistance(rp), 500.0, 1e-9);
    }
}

// ── ecmath — capacitors ───────────────────────────────────────────────────────

TEST(EcMath, CapacitorsParallel) {
    Capacitor ca(cfg("CA", "T", "T", "", kDefaultRating), Capacitor::Type::Ceramic, 10e-6);
    Capacitor cb(cfg("CB", "T", "T", "", kDefaultRating), Capacitor::Type::Ceramic, 40e-6);
    vector<const Capacitor*> cp = {&ca, &cb};
    // 10u + 40u = 50u
    EXPECT_NEAR(ecmath::computeCapacitorsInParallel(cp), 50e-6, 1e-18);
}

TEST(EcMath, CapacitorsSeries) {
    Capacitor ca(cfg("CA2", "T", "T", "", kDefaultRating), Capacitor::Type::Ceramic, 10e-6);
    Capacitor cb(cfg("CB2", "T", "T", "", kDefaultRating), Capacitor::Type::Ceramic, 40e-6);
    vector<const Capacitor*> cp = {&ca, &cb};
    // 1/(1/10u + 1/40u) = 8u
    EXPECT_NEAR(ecmath::computeCapacitorsInSeries(cp), 8e-6, 1e-18);
}

// ── ecmath — inductors ────────────────────────────────────────────────────────

TEST(EcMath, InductorsSeries) {
    Inductor la(cfg("LA", "T", "T", "", kDefaultRating), 200e-6);
    Inductor lb(cfg("LB", "T", "T", "", kDefaultRating), 300e-6);
    vector<const Inductor*> lp = {&la, &lb};
    // 200u + 300u = 500u
    EXPECT_NEAR(ecmath::computeInductorsInSeries(lp), 500e-6, 1e-18);
}

TEST(EcMath, InductorsParallel) {
    Inductor la(cfg("LA2", "T", "T", "", kDefaultRating), 200e-6);
    Inductor lb(cfg("LB2", "T", "T", "", kDefaultRating), 300e-6);
    vector<const Inductor*> lp = {&la, &lb};
    // 1/(1/200u + 1/300u) = 120u
    EXPECT_NEAR(ecmath::computeInductorsInParallel(lp), 120e-6, 1e-18);
}
